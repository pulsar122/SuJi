#include "SuJi_glo.h"

void output_date ( FILE_INFO *par, char *ZStr )
{
	OBJECT *tree;
	char trenner;

	mt_rsrc_gaddr(0,LANGUAGE_SETTING,&tree,&global);
	trenner=tree[LS_DM_TRENNER].ob_spec.tedinfo->te_ptext[0];

	if(tree[LS_DAY_MONTH].ob_state & SELECTED)
	{
		sprintf(ZStr,"%02d%c%02d%c%04d",
			(par->date & 0x001f),
			trenner,
			(par->date & 0x01e0)/0x0020,
			trenner,
			(par->date & 0xfe00)/0x0200+1980);
	}
	else
	{
		sprintf(ZStr,"%02d%c%02d%c%04d",
			(par->date & 0x01e0)/0x0020,
			trenner,
			(par->date & 0x001f),
			trenner,
			(par->date & 0xfe00)/0x0200+1980);
	}
}

int writeline(FILE_INFO *par,long max, long max_pfad, FILE *fp)
{
	char ZStr[1024], Zahl[20], *c;
	int k;
	long len;

	if ( strlen ( config.clipboard_format ) != 0 )
	{
		c = config.clipboard_format;
		k = 0;
		ZStr[0] = '\0';
		
		for (; *c != '\0'; c++ )
		{
			if ( *c == '%' )
			{
				len = strlen ( ZStr );
				c++;
				if ( *c == '\0' )
					break;
				else
				{
					if ( *c == 'N' )
						sprintf( &ZStr[len], "%-*s", (int)max, par->name);
					else if ( *c == 'n' )
						strcat ( ZStr, par->name);
					else if ( *c == 'S' || *c == 's' )
						{
							ultoa( par->size, Zahl, 10);
							sprintf( &ZStr[len], "%8s", Zahl);
						}
					else if ( *c == 'T' || *c == 't' )
						{
							sprintf(Zahl,"%02d:%02d:%02d",
								(par->time & 0xf800)/0x0800,
								(par->time & 0x07e0)/0x0020,
								(par->time & 0x001f)*2);
							sprintf( &ZStr[len], "%8s", Zahl);
						}
					else if ( *c == 'D' || *c == 'd' )
						{
							output_date ( par, Zahl );
							sprintf( &ZStr[len], "%8s", Zahl);
						}
					else if ( *c == 'F' || *c == 'f' )
						{
							Zahl[0]=(par->attrib & FA_READONLY) ? 'R' : '-';
							Zahl[1]=(par->attrib & FA_HIDDEN) ? 'H' : '-';
							Zahl[2]=(par->attrib & FA_SYSTEM) ? 'S' : '-';
							Zahl[3]=(par->attrib & FA_VOLUME) ? 'V' : '-';
							Zahl[4]=(par->attrib & FA_SUBDIR) ? 'D' : '-';
							Zahl[5]=(par->attrib & FA_ARCHIVE) ? 'A' : '-';
							Zahl[6]=(par->attrib & FA_LINK) ? 'L' : '-';
							Zahl[7]='\0';
							strcat( ZStr, Zahl);
						}
					else if ( *c == 'O' )
						sprintf( &ZStr[len], "%-*s", (int)max_pfad, par->pfad );
					else if ( *c == 'o' )
						strcat ( ZStr, par->pfad );
					else if ( *c == 'R' )
						strcat ( ZStr, "\n" );
					else if ( *c == '%' )
						strcat ( ZStr, "%" );
				}
			}
			else
			{
				len = strlen ( ZStr );
				ZStr[len++] = *c;
				ZStr[len] = '\0';
			}
		}
		fprintf(fp, ZStr);
		fprintf(fp, "\n");
	}
	else
	{
		k=0;
		if(config.clipboard_name)
		{
			k++;
			if(fprintf(fp,"%-*s",(int)max,par->name)==EOF)
				return 0;
		}
	
		if(k)
			fprintf(fp," ");
		
		if(config.clipboard_size)
		{
			k++;
			ultoa(par->size,ZStr,10);
			if(fprintf(fp,"%8s",ZStr)==EOF)
				return 0;
		}
	
		if(k)
			fprintf(fp," ");
	
		if(config.clipboard_time)
		{
			k++;
			sprintf(ZStr,"%02d:%02d:%02d",
				(par->time & 0xf800)/0x0800,
				(par->time & 0x07e0)/0x0020,
				(par->time & 0x001f)*2);
			if(fprintf(fp,"%s",ZStr)==EOF)
				return 0;
		}
	
		if(k)
			fprintf(fp," ");
	
		if(config.clipboard_date)
		{ /* Prfen, ob Tag oder Monat vorne stehen soll */
			OBJECT *tree;
			char trenner;
	
			k++;
			mt_rsrc_gaddr(0,LANGUAGE_SETTING,&tree,&global);
			trenner=tree[LS_DM_TRENNER].ob_spec.tedinfo->te_ptext[0];
	
			if(tree[LS_DAY_MONTH].ob_state & SELECTED)
			{
				sprintf(ZStr,"%02d%c%02d%c%04d",
					(par->date & 0x001f),
					trenner,
					(par->date & 0x01e0)/0x0020,
					trenner,
					(par->date & 0xfe00)/0x0200+1980);
			}
			else
			{
				sprintf(ZStr,"%02d%c%02d%c%04d",
					(par->date & 0x01e0)/0x0020,
					trenner,
					(par->date & 0x001f),
					trenner,
					(par->date & 0xfe00)/0x0200+1980);
			}
			if(fprintf(fp,"%s",ZStr)==EOF)
				return 0;
		}
	
		if(k)
			fprintf(fp," ");
	
		if(config.clipboard_flags)
		{
			k++;
			ZStr[0]=(par->attrib & FA_READONLY) ? 'R' : '-';
			ZStr[1]=(par->attrib & FA_HIDDEN) ? 'H' : '-';
			ZStr[2]=(par->attrib & FA_SYSTEM) ? 'S' : '-';
			ZStr[3]=(par->attrib & FA_VOLUME) ? 'V' : '-';
			ZStr[4]=(par->attrib & FA_SUBDIR) ? 'D' : '-';
			ZStr[5]=(par->attrib & FA_ARCHIVE) ? 'A' : '-';
			Zahl[6]=(par->attrib & FA_LINK) ? 'L' : '-';
			ZStr[7]='\0';
			if(fprintf(fp,"%s",ZStr)==EOF)
				return 0;
		}
	
		if(k)
			fprintf(fp," ");
	
		if(config.clipboard_origin)
		{
			k++;
			if(fprintf(fp,"%-*s",(int)max_pfad,par->pfad)==EOF)
				return 0;
		}
	
		if(k)
			fprintf(fp," ");
	
		if(config.clipboard_origin_name)
		{
			k++;
			if(fprintf(fp,"%s%s",par->pfad,par->name)==EOF)
				return 0;
		}

		if(k)
			fprintf(fp,"\n");
	}

	return 1;
}

void write_to_scrap (void )
{
	unsigned long l;
	char file[1024];
	char mask[1024];
	
	if(mt_scrp_read(file,&global))
	{
		FILE *fp;
	
		strcpy(mask,file);
		strcat(mask,"SCRAP.*");
	
		while(!Fdelete(mask));	/* SCRAP.* delete */
	
		strcpy(mask,file);
		strcat(mask,"scrap.*");
	
		while(!Fdelete(mask));	/* scrap.* delete */
	
		strcpy(mask,file);
		strcat(mask,"scrap.txt");
	
/*						handle_option(); */
		
		fp=fopen(mask,"w");
		if(fp)
		{
			int found=0;
			long max=0, max_pfad=0;
	
			for(l=0;l<finfos;l++)
			{
				FILE_INFO *par;
	
				par=get_from_list(l+1);
				if(par && strlen(par->name)>max )
					max=strlen(par->name);
				if(par && strlen(par->pfad)>max_pfad )
					max_pfad=strlen(par->pfad);
			}
	
			for(l=0;l<finfos;l++)
			{
				FILE_INFO *par;
	
				par=get_from_list(l+1);
				if(par && par->selected)
				{
					found=1;
					if(!writeline(par,max,max_pfad,fp))
					{
						mt_rsrc_gaddr(5,ERR_WRITE_SCRAP,&alert,&global);
						mt_form_alert(1,alert,&global);
						l=finfos;
						fclose(fp);
						fp=NULL;
					}
				}
			}
	
			if(!found)
			{
				for(l=0;l<finfos;l++)
				{
					FILE_INFO *par;
	
					par=get_from_list(l+1);
					if(par)
					{
						if(!writeline(par,max,max_pfad,fp))
						{
							mt_rsrc_gaddr(5,ERR_WRITE_SCRAP,&alert,&global);
							mt_form_alert(1,alert,&global);
							l=finfos;
							fclose(fp);
							fp=NULL;
						}
					}
				}
			}
	
			if(fp)
			{
				int ret;
	
				fclose(fp);
	
				if(mt_appl_getinfo(10,&ret,NULL,NULL,NULL,&global) && (ret & 0x00ff)>=7)
				{
					int msg[8];
	
					msg[0]=SC_CHANGED;
					msg[1]=ap_id;
					msg[2]=0;
					msg[3]=2;
					msg[4]='.t';
					msg[5]='xt';
					msg[6]=msg[7]=0;
	
					mt_shel_write(7,0,0,(void *)msg,NULL,&global);
	
					msg[0]=SH_WDRAW;
	
					if(mask[1]==':' && mask[0]>='A' && mask[0]<='Z')
						msg[3]=mask[0]-'A';
					else if(mask[1]==':' && mask[0]>='a' && mask[0]<='z')
						msg[3]=mask[0]-'a';
					else
						msg[3]=-1;
	
					msg[4]=msg[5]=msg[6]=msg[7]=0;
	
					mt_appl_write(0,16,msg,&global);
				}
			}
		}
		else
		{
			mt_rsrc_gaddr(5,ERR_WRITE_SCRAP,&alert,&global);
			mt_form_alert(1,alert,&global);
		}
	}
	else
	{
		mt_rsrc_gaddr(5,ERR_NO_SCRAP,&alert,&global);
		mt_form_alert(1,alert,&global);
	}
}
