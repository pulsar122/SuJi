/*
	Tabulatorweite: 2
	Kommentare ab: Spalte 60											*Spalte 60*
	
	From WDIALOG modify for SuJi
*/

/*----------------------------------------------------------------------------------------*/ 
/* Globale Includes																																				*/
/*----------------------------------------------------------------------------------------*/ 
#include "SuJi_glo.h"



/*----------------------------------------------------------------------------------------*/ 
/* Lokale Includes																																				*/
/*----------------------------------------------------------------------------------------*/ 

/*----------------------------------------------------------------------------------------*/ 

typedef struct
{
	LONG	id;
	LONG	value;
} COOKIE;

static USERBLK	*substitute_ublks;
static OBJECT	*radio_slct;
static OBJECT	*radio_deslct;
static WORD	radio_bgcol;
static WORD	magic_version;

static WORD	cdecl check_button( PARMBLK *parmblock );
static WORD	cdecl radio_button( PARMBLK *parmblock );
static WORD	cdecl group( PARMBLK *parmblock );
static WORD	cdecl title( PARMBLK *parmblock );
static void	userdef_text( WORD x, WORD y, BYTE *string );

/*----------------------------------------------------------------------------------------*/ 
/* Informationen �ber die AES-Funktionen zur�ckliefern												*/
/* Funktionsergebnis:	diverse Flags																		*/
/*	font_id:					ID des AES-Fonts																	*/
/*	font_height:			H�he des AES-Fonts (f�r vst_height())										*/
/*	hor_3d:					zus�tzlicher horizontaler beidseitiger Rand f�r 3D-Objekte			*/
/*	ver_3d:					zus�tzlicher vertikaler beidseitiger Rand f�r 3D-Objekte				*/
/*----------------------------------------------------------------------------------------*/ 
WORD	get_aes_info( WORD *font_id, WORD *font_height, WORD *hor_3d, WORD *ver_3d, GlobalArray *global )
{
	MAGX_COOKIE	*magic;
	long cookie;
	WORD	work_out[57];
	WORD	attrib[10];
	WORD	pens;
	WORD	flags;
	
	vq_extnd( vdi_h, 0, work_out );
	vqt_attributes( vdi_h, attrib );

	flags = 0;
	pens = work_out[13];													/* Anzahl der Farbstifte */
	*font_id = attrib[0];												/* Standardfont */
	*font_height = attrib[7];											/* Standardh�he */
	*hor_3d = 0;
	*ver_3d = 0;
	radio_bgcol = 0;

	if ( mt_appl_find( "?AGI", global ) == 0 )									/* appl_getinfo() vorhanden? */
		flags |= GAI_INFO;

	if ( global->ap_version >= 0x0401 )											/* mindestens AES 4.01? */
		flags |= GAI_INFO;

	if(!Ash_getcookie((long) 'MagX',&cookie))
		 magic = NULL;
	else
		magic = (MAGX_COOKIE *) cookie;

	magic_version = 0;
	
	if ( magic )															/* MagiC vorhanden? */
	{
		if ( magic->aesvars )											/* MagiC-AES aktiv? */
		{
			magic_version = magic->aesvars->version;				/* MagiC-Versionsnummer */
			flags |= GAI_MAGIC + GAI_INFO;
		}
	}
		
	if ( flags & GAI_INFO )												/* ist appl_getinfo() vorhanden? */
	{
		WORD	ag1;
		WORD	ag2;
		WORD	ag3;
		WORD	ag4;

		if ( mt_appl_getinfo( 0, &ag1, &ag2, &ag3, &ag4, global ))			/* Unterfunktion 0, Fonts */
		{
			*font_id = ag2;
			*font_height = ag1;
		}

		if ( mt_appl_getinfo( 2, &ag1, &ag2, &ag3, &ag4, global ) && ag3 )	/* Unterfunktion 2, Farben */
			flags |= GAI_CICN;

		if ( mt_appl_getinfo( 7, &ag1, &ag2, &ag3, &ag4, global ))			/* Unterfunktion 7 */
			flags |= ag1 & 0x0f;

		if ( mt_appl_getinfo( 12, &ag1, &ag2, &ag3, &ag4, global ) && ( ag1 & 8 ))	/* AP_TERM? */
			flags |= GAI_APTERM;

		if ( mt_appl_getinfo( 13, &ag1, &ag2, &ag3, &ag4, global ))		/* Unterfunktion 13, Objekte */
		{
			if ( ag4 & 0x04 )																						/* WHITEBAK steuert Unterstriche ? */
				flags |= GAI_WHITEBAK;

			if ( ag4 & 0x08 )																						/* G_SHORTCUT unterst�tzt ? */
				flags |= GAI_GSHORTCUT;
				
			if ( ag1 && ag2 )												/* 3D-Objekte und objc_sysvar() vorhanden? */
			{
				if ( mt_objc_sysvar( 0, AD3DVALUE, 0, 0, hor_3d, ver_3d, global ))	/* 3D-Look eingeschaltet? */
				{
					if ( pens >= 16 )										/* mindestens 16 Farben? */
					{
						WORD	dummy;
						
						flags |= GAI_3D;
						mt_objc_sysvar( 0, BACKGRCOL, 0, 0, &radio_bgcol, &dummy, global );
					}
				}
			}
		}
	}
	
	return( flags );
}

/*----------------------------------------------------------------------------------------*/ 
/* Horizontale und Vertikale Vergr��erung und Verschiebung von 3D-Objekten kompensieren	*/
/* Funktionsergebnis:	-																						*/
/* objs:						Zeiger auf die Objekte															*/
/*	no_objs:					Anzahl der Objekte																*/
/*	hor_3d:					zus�tzlicher horizontaler beidseitiger Rand f�r 3D-Objekte			*/
/*	ver_3d:					zus�tzlicher vertikaler beidseitiger Rand f�r 3D-Objekte				*/
/*----------------------------------------------------------------------------------------*/ 
void	adapt3d_rsrc( OBJECT *objs, UWORD no_objs, WORD hor_3d, WORD ver_3d )
{
	while ( no_objs > 0 )
	{
		if ( objs->ob_flags & FL3DIND )								/* Indicator oder Activator? */
		{
			objs->ob_x += hor_3d;
			objs->ob_y += ver_3d;
			objs->ob_width -= 2 * hor_3d;
			objs->ob_height -= 2 * ver_3d;
		}
		objs++;																/* n�chstes Objekt */
		no_objs--;
	}
}

/*----------------------------------------------------------------------------------------*/ 
/* 3D-Flags l�schen und Objektgr��en anpassen, wenn 3D-Look ausgeschaltet ist					*/
/* Funktionsergebnis:	-																						*/
/* objs:						Zeiger auf die Objekte															*/
/*	no_objs:					Anzahl der Objekte																*/
/*	ftext_to_fboxtext:	Flag daf�r, da� FTEXT-Objekte in FBOXTEXT umgewandelt werden		*/
/*----------------------------------------------------------------------------------------*/ 
void	no3d_rsrc( OBJECT *objs, UWORD no_objs, WORD ftext_to_fboxtext )
{
	radio_bgcol = 0;														/* Annahme: Hintergrundfarbe bei 2D ist wei� */
	
	while ( no_objs > 0 )
	{
		if ( ftext_to_fboxtext )										/* FTEXT-Objekte in FBOXTEXT umwandeln? */
		{
			if (( objs->ob_type & 0xff ) == G_FTEXT )				/* FTEXT-Objekt? */
			{
				if ( objs->ob_flags & FL3DMASK )						/* mit 3D-Optik? */
				{
					if ( objs->ob_spec.tedinfo->te_thickness == -2 )
					{
						objs->ob_state |= OUTLINED;
						objs->ob_spec.tedinfo->te_thickness = -1;
						objs->ob_type = G_FBOXTEXT;
					}
				}
			}
		}

		objs->ob_flags &= ~FL3DMASK;									/* 3D-Flags l�schen */
		
		objs++;																/* n�chstes Objekt */
		no_objs--;
	}
}

/*----------------------------------------------------------------------------------------*/ 
/* Testen, ob ein Objekt USERDEF ist und ob es einen G_STRING-Titel ersetzt					*/
/* Funktionsergebnis:	Zeiger auf den String oder 0L, wenn es kein USERDEF-Titel ist		*/
/*	obj:						Zeiger auf das Objekt															*/
/*----------------------------------------------------------------------------------------*/ 
BYTE	*is_userdef_title( OBJECT *obj )
{
	if ( substitute_ublks )												/* werden MagiC-Objekte ersetzt? */
	{
		if ( obj->ob_type == G_USERDEF )
		{
			USERBLK	*ublk;
			
			ublk = obj->ob_spec.userblk;
			
			if ( ublk->ub_code == title )								/* ersetzte �berschrift? */
				return((BYTE *) ublk->ub_parm );						/* Zeiger auf den String zur�ckliefern */
		}
	}
	return( 0L );
}

/*----------------------------------------------------------------------------------------*/ 
/* MagiC-Objekte durch USERDEFs ersetzen																	*/
/* Funktionsergebnis:	-																						*/
/* objs:						Zeiger auf die Objekte															*/
/*	no_objs:					Anzahl der Objekte																*/
/*	aes_flags:				Informationen �ber das AES														*/
/*	rslct:					Zeiger auf Image f�r selektierten Radio-Button							*/
/*	rdeslct:					Zeiger auf Image f�r deselektierten Radio-Button						*/
/*----------------------------------------------------------------------------------------*/ 
void	substitute_objects( OBJECT *objs, UWORD no_objs, WORD aes_flags, OBJECT *rslct, OBJECT *rdeslct )
{
	OBJECT	*obj;
	unsigned int	i;
	int no_subs;
	
	if (( aes_flags & GAI_MAGIC ) && ( magic_version >= 0x0300 ))	/* MagiC-AES? */
	{
		substitute_ublks = 0L;
		return;	
	}
	
	if ( aes_flags & GAI_WHITEBAK )
	{
		substitute_ublks = 0L;
		return;	
	}
	
	obj = objs;																/* Zeiger auf die Objekte */
	i = no_objs;															/* Anzahl der Objekte im gesamten Resource */

	no_subs = 0;

	while ( i > 0 )
	{
		if (( obj->ob_state & WHITEBAK ) && ( obj->ob_state & 0x8000 ))	/* MagiC-Objekt? */
		{
			switch ( obj->ob_type & 0xff )
			{
				case	G_BUTTON:											/* Checkbox, Radiobutton oder Gruppenrahmen? */
				{
					no_subs++;
					break;
				}
				case	G_STRING:											/* �berschrift? */
				{
					if (( obj->ob_state & 0xff00 ) == 0xff00L )	/* Unterstreichung auf voller L�nge? */
						no_subs++;
					break;
				}
			}
		}
		obj++;																/* n�chstes Objekt */
		i--;
	}

	if ( no_subs )															/* sind MagiC-Objekte vorhanden? */
	{
		substitute_ublks = Malloc( no_subs * sizeof( USERBLK ));
		radio_slct = rslct;
		radio_deslct = rdeslct;
		
		if ( substitute_ublks )											/* Speicher vorhanden? */
		{
			USERBLK	*tmp;
			
			tmp = substitute_ublks;
	
			obj = objs;														/* Zeiger auf die Objekte */
			i = no_objs;													/* Anzahl der Objekte im gesamten Resource */
			
			while ( i > 0 )
			{
				WORD	type;
				UWORD	state;
				
				type = obj->ob_type & 0x00ff;
				state = (UWORD) obj->ob_state;
				
				if (( state & WHITEBAK ) && ( state & 0x8000 ))	/* MagiC-Objekt? */
				{
					state &= 0xff00;										/* nur das obere Byte ist interessant */

					if ( aes_flags & GAI_MAGIC )						/* altes MagiC-AES? */
					{
						if (( type == G_BUTTON ) && ( state == 0xfe00 ))	/* Gruppenrahmen? */
						{
							tmp->ub_parm = (LONG) obj->ob_spec.free_string;	/* Zeiger auf den Text */
							tmp->ub_code = group;
						
							obj->ob_type = G_USERDEF;
							obj->ob_flags &= ~FL3DMASK;				/* 3D-Flags l�schen */
							obj->ob_spec.userblk = tmp;				/* Zeiger auf den USERBLK */
							
							tmp++;
						}
					}
					else														/* TOS-AES oder sonstiges */
					{
						switch ( type )
						{
							case	G_BUTTON:								/* Checkbox, Radiobutton oder Gruppenrahmen? */
							{
								tmp->ub_parm = (LONG) obj->ob_spec.free_string;	/* Zeiger auf den Text */
			
								if ( state == 0xfe00 )					/* Gruppenrahmen? */
									tmp->ub_code = group;
								else if ( obj->ob_flags & RBUTTON )	/* Radio-Button? */
									tmp->ub_code = radio_button;
								else											/* Check-Button */
									tmp->ub_code = check_button;

								obj->ob_type = G_USERDEF;
								obj->ob_flags &= ~FL3DMASK;			/* 3D-Flags l�schen */
								obj->ob_spec.userblk = tmp;			/* Zeiger auf den USERBLK */
								
								tmp++;
								break;
							}
							case	G_STRING:								/* �berschrift? */
							{
								if ( state == 0xff00 )					/* Unterstreichung auf voller L�nge? */
								{
									tmp->ub_parm = (LONG) obj->ob_spec.free_string;	/* Zeiger auf den Text */
									tmp->ub_code = title;
									obj->ob_type = G_USERDEF;
									obj->ob_flags &= ~FL3DMASK;		/* 3D-Flags l�schen */
									obj->ob_spec.userblk = tmp;		/* Zeiger auf den USERBLK */
									tmp++;
								}
								break;
							}
						}
					}
				}	
				obj++;														/* n�chstes Objekt */
				i--;
			}
		}
	}
}

/*----------------------------------------------------------------------------------------*/ 
/* Speicher f�r Resource-Anpassung freigeben																*/
/* Funktionsresultat:	-																						*/
/*----------------------------------------------------------------------------------------*/ 
void	substitute_free( void )
{
	if ( substitute_ublks )												/* Speicher vorhanden? */
		Mfree( substitute_ublks );
		
	substitute_ublks = 0L;
}

/*----------------------------------------------------------------------------------------*/ 
/* USERDEF-Funktion f�r Check-Button																		*/
/* Funktionsresultat:	nicht aktualisierte Objektstati												*/
/* parmblock:				Zeiger auf die Parameter-Block-Struktur									*/
/*----------------------------------------------------------------------------------------*/ 
static WORD	cdecl check_button( PARMBLK *parmblock )
{
	WORD	rect[4];
	WORD	clip[4];
	WORD	xy[10];
	BYTE	*string;
	
	string = (BYTE *) parmblock->pb_parm;

	*(GRECT *) clip = *(GRECT *) &parmblock->pb_xc;				/* Clipping-Rechteck... */
	clip[2] += clip[0] - 1;
	clip[3] += clip[1] - 1;
	vs_clip( vdi_h, 1, clip );									/* Zeichenoperationen auf gegebenen Bereich beschr�nken */

	*(GRECT *) rect = *(GRECT *) &parmblock->pb_x;				/* Objekt-Rechteck... */
	rect[2] = rect[0] + phchar - 2;
	rect[3] = rect[1] + phchar - 2;

	vswr_mode( vdi_h, 1 );										/* Ersetzend */

	vsl_color( vdi_h, 1 );										/* schwarz */
	xy[0] = rect[0];
	xy[1] = rect[1];
	xy[2] = rect[2];
	xy[3] = rect[1];
	xy[4] = rect[2];
	xy[5] = rect[3];
	xy[6] = rect[0];
	xy[7] = rect[3];
	xy[8] = rect[0];
	xy[9] = rect[1];
	v_pline( vdi_h, 5, xy );										/* schwarzen Rahmen zeichnen */

	vsf_color( vdi_h, 0 );										/* wei� */
	
	xy[0] = rect[0] + 1;
	xy[1] = rect[1] + 1;
	xy[2] = rect[2] - 1;
	xy[3] = rect[3] - 1;
	vr_recfl( vdi_h, xy );										/* wei�e Box zeichnen */

	if ( parmblock->pb_currstate & SELECTED )
	{
		parmblock->pb_currstate &= ~SELECTED;						/* Bit l�schen */
		
		vsl_color( vdi_h, 1 );									/* schwarz - f�r das Kreuz */
		xy[0] = rect[0] + 2;
		xy[1] = rect[1] + 2;
		xy[2] = rect[2] - 2;
		xy[3] = rect[3] - 2;
		v_pline( vdi_h, 2, xy );
		
		xy[1] = rect[3] - 2;
		xy[3] = rect[1] + 2;
		v_pline( vdi_h, 2, xy );
	}
	userdef_text( parmblock->pb_x + phchar + pwchar, parmblock->pb_y, string );

	return( parmblock->pb_currstate );
}

/*----------------------------------------------------------------------------------------*/ 
/* USERDEF-Funktion f�r Radio-Button																		*/
/* Funktionsresultat:	nicht aktualisierte Objektstati												*/
/* parmblock:				Zeiger auf die Parameter-Block-Struktur									*/
/*----------------------------------------------------------------------------------------*/ 
static WORD	cdecl radio_button( PARMBLK *parmblock )
{
	BITBLK	*image;
	MFDB	src;
	MFDB	des;
	WORD	clip[4];
	WORD	xy[8];
	WORD	image_colors[2];
	BYTE	*string;

	*(GRECT *) clip = *(GRECT *) &parmblock->pb_xc;				/* Clipping-Rechteck... */
	clip[2] += clip[0] - 1;
	clip[3] += clip[1] - 1;
	vs_clip( vdi_h, 1, clip );									/* Zeichenoperationen auf gegebenen Bereich beschr�nken */

	string = (BYTE *) parmblock->pb_parm;

	if ( parmblock->pb_currstate & SELECTED )						/* Selektion? */
	{
		parmblock->pb_currstate &= ~SELECTED;						/* Bit l�schen */

		image = radio_slct->ob_spec.bitblk;
	}
	else
		image = radio_deslct->ob_spec.bitblk;
		
	src.fd_addr = image->bi_pdata;
	src.fd_w = image->bi_wb * 8;
	src.fd_h = image->bi_hl;
	src.fd_wdwidth = image->bi_wb / 2;
	src.fd_stand = 0;
	src.fd_nplanes = 1;
	src.fd_r1 = 0;
	src.fd_r2 = 0;
	src.fd_r3 = 0;

	des.fd_addr = 0L;

	xy[0] = 0;
	xy[1] = 0;
	xy[2] = src.fd_w - 1;
	xy[3] = src.fd_h - 1;
	xy[4] = parmblock->pb_x;
	xy[5] = parmblock->pb_y;
	xy[6] = xy[4] + xy[2];
	xy[7] = xy[5] + xy[3];

	image_colors[0] = 1;													/* schwarz als Vordergrundfarbe */
	image_colors[1] = radio_bgcol;									/* Hintergrundfarbe */

	vrt_cpyfm( vdi_h, MD_REPLACE, xy, &src, &des, image_colors );
	userdef_text( parmblock->pb_x + phchar + pwchar, parmblock->pb_y, string );

	return( parmblock->pb_currstate );
}

/*----------------------------------------------------------------------------------------*/ 
/* USERDEF-Funktion f�r Gruppen-Rahmen																		*/
/* Funktionsresultat:	nicht aktualisierte Objektstati												*/
/* parmblock:				Zeiger auf die Parameter-Block-Struktur									*/
/*----------------------------------------------------------------------------------------*/ 
static WORD	cdecl group( PARMBLK *parmblock )
{
	WORD	clip[4];
	WORD	obj[4];
	WORD	xy[12];
	BYTE	*string;

	string = (BYTE *) parmblock->pb_parm;

	*(GRECT *) &clip = *(GRECT *) &parmblock->pb_xc;			/* Clipping-Rechteck... */
	clip[2] += clip[0] - 1;
	clip[3] += clip[1] - 1;
	vs_clip( vdi_h, 1, clip );									/* Zeichenoperationen auf gegebenen Bereich beschr�nken */

	vswr_mode( vdi_h, MD_TRANS );
	vsl_color( vdi_h, 1 );
	vsl_type( vdi_h, 1 );

	*(GRECT *) obj = *(GRECT *) &parmblock->pb_x;				/* Objekt-Rechteck... */
	obj[2] += obj[0] - 1;
	obj[3] += obj[1] - 1;

	xy[0] = obj[0] + pwchar;
	xy[1] = obj[1] + phchar / 2;
	xy[2] = obj[0];
	xy[3] = xy[1];
	xy[4] = obj[0];
	xy[5] = obj[3];
	xy[6] = obj[2];
	xy[7] = obj[3];
	xy[8] = obj[2];
	xy[9] = xy[1];
	xy[10] = (WORD) ( xy[0] + strlen( string ) * pwchar );
	xy[11] = xy[1];
	
	v_pline( vdi_h, 6, xy );

	userdef_text( obj[0] + pwchar, obj[1], string );

	return( parmblock->pb_currstate );
}

/*----------------------------------------------------------------------------------------*/ 
/* USERDEF-Funktion f�r �berschrift																			*/
/* Funktionsresultat:	nicht aktualisierte Objektstati												*/
/* parmblock:				Zeiger auf die Parameter-Block-Struktur									*/
/*----------------------------------------------------------------------------------------*/ 
static WORD	cdecl title( PARMBLK *parmblock )
{
	WORD	clip[4];
	WORD	xy[4];
	BYTE	*string;

	string = (BYTE *) parmblock->pb_parm;

	*(GRECT *) &clip = *(GRECT *) &parmblock->pb_xc;			/* Clipping-Rechteck... */
	clip[2] += clip[0] - 1;
	clip[3] += clip[1] - 1;
	vs_clip( vdi_h, 1, clip );									/* Zeichenoperationen auf gegebenen Bereich beschr�nken */

	vswr_mode( vdi_h, MD_TRANS );
	vsl_color( vdi_h, 1 );
	vsl_type( vdi_h, 1 );

	xy[0] = parmblock->pb_x;
	xy[1] = parmblock->pb_y + parmblock->pb_h - 1;
	xy[2] = parmblock->pb_x + parmblock->pb_w - 1;
	xy[3] = xy[1];
	v_pline( vdi_h, 2, xy );

	userdef_text( parmblock->pb_x, parmblock->pb_y, string );

	return( parmblock->pb_currstate );
}

static void	userdef_text( WORD x, WORD y, BYTE *string )
{
	WORD	tmp;
	
	vswr_mode( vdi_h, MD_TRANS );
	vst_font( vdi_h, aes_font );							/* Font einstellen */
	vst_color( vdi_h, 1 );										/* schwarz */
	vst_effects( vdi_h, 0 );									/* keine Effekte */
	vst_alignment( vdi_h, 0, 5, &tmp, &tmp );	/* an der Zeichenzellenoberkante ausrichten */
	vst_height( vdi_h, aes_height, &tmp, &tmp, &tmp, &tmp );
	
	v_gtext( vdi_h, x, y, string );
}
