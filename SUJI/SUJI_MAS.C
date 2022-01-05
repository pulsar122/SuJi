#include "SuJi_glo.h"

int doppelklick;

void simuliere_taste_button(EVNT *events)
{
	int msg[8];

	msg[1]=ap_id;
	msg[2]=0;

	if(events->mwhich & MU_KEYBD)
	{
		msg[0]=AV_SENDKEY;
		msg[3]=events->kstate;
		msg[4]=events->key;

		mt_appl_write(ap_id,16,msg,&global);
	}
	if(events->mwhich & MU_BUTTON)
	{
		msg[0]=AV_SENDCLICK;

		msg[3]=events->mx;
		msg[4]=events->my;
		msg[5]=events->mbutton;
		msg[6]=events->kstate;
		msg[7]=events->mclicks;

		mt_appl_write(ap_id,16,msg,&global);
	}

	events->mwhich&=~(MU_KEYBD|MU_BUTTON);
}


int thread_may_exit=1;

/* Return:  0 = 																													*/
/*				 -1 = ESC																												*/
/*				 -2 = CNTRL_Q, CNTRL_U																					*/
/*				 -3 = CNTRL_F																										*/

int master_event(unsigned int end_if_timer)
{
	do {
		MOBLK m1blk={0, 0, 0 ,0, 0};
		MOBLK m2blk={0, 0, 0 ,0, 0};

		events.mwhich = 0;

		mt_EVNT_multi(MU_KEYBD|MU_BUTTON|MU_MESAG|
								(end_if_timer ? MU_TIMER : 0)
				,0x102,3,0
				,&m1blk,&m2blk,
				(unsigned long)end_if_timer-1,
				&events,&global);
/*
MsgAndLF ( "master_event:" );
MsgEVNT ( &events );
*/
if ( events.mwhich & MU_MESAG )
/* 	MsgShowMessage ( &events.msg[0] ); */

		if(events.mwhich & MU_MESAG)
		{
			switch(events.msg[0])
			{
				case THR_EXIT:
					if(events.msg[3]==thread_id_inhalt)
					{
						thread_id_inhalt=0;
						simuliere_taste_button(&events);
						thread_ret=events.msg[5];

						if(thread_may_exit)
							return 0;
						else
							mt_appl_write(ap_id,16,events.msg,&global);
					}
					break;
				case WM_SHADED:
					shaded_or_iconified|=1;
					break;
				case WM_UNSHADED:
					shaded_or_iconified&=~1;
					break;
				case WM_ICONIFY:
				case WM_ALLICONIFY:
					mt_wind_set_grect(events.msg[3],WF_ICONIFY,(GRECT *)&events.msg[4],&global);
					full.g_w=full.g_h=-1;
					shaded_or_iconified|=2;
					break;
				case WM_UNICONIFY:
					mt_wind_set_grect(events.msg[3],WF_UNICONIFY,(GRECT *)&events.msg[4],&global);
					calc_slider();
					shaded_or_iconified&=~2;
					break;
				case WM_TOPPED:
					mt_wind_set(events.msg[3],WF_TOP,0,0,0,0,&global);
					break;
				case WM_MOVED:
					mt_wind_set_grect(events.msg[3],WF_CURRXYWH,(GRECT *)&events.msg[4],&global);
					break;
				case WM_FULLED:
				{
					GRECT work;

					if(full.g_h==-1 || full.g_w==-1)
					{
						GRECT to;

						mt_wind_get_grect(0,WF_WORKXYWH,&work,&global);

						mt_wind_get_grect(window_handle,WF_CURRXYWH,&full,&global);

						mt_wind_get_grect(window_handle,WF_WORKXYWH,&to,&global);

						to.g_x=to.g_x+to.g_w/2;
						to.g_y=to.g_y+to.g_h/2;

						to.g_w=6;
						for(i=0;i<6;i++)
						{
							if(show_row[i])
								to.g_w+=max_breite[i];
						}

						to.g_h=work.g_h;

						if(((unsigned long) to.g_h)>=finfos*line_height+top_height)
							to.g_h=(int) ((finfos>2 ? finfos : 3)*line_height+top_height);

						to.g_x-=to.g_w/2;
						to.g_y-=to.g_h/2;

						mt_wind_calc_grect(WC_BORDER,WIND_KIND,&to,&to,&global);

						if(to.g_x<work.g_x)
							to.g_x=work.g_x;
						if(to.g_y<work.g_y)
							to.g_y=work.g_y;
						if(to.g_x+to.g_w>work.g_x+work.g_w)
							to.g_w=work.g_x+work.g_w-to.g_x;
						if(to.g_y+to.g_h>work.g_y+work.g_h)
							to.g_h=work.g_y+work.g_h-to.g_y;

						mt_wind_calc_grect(WC_WORK,WIND_KIND,&to,&to,&global);

						to.g_h=(to.g_h/line_height)*line_height;
	
						lines_to_show=to.g_h/line_height-1;
	
						to.g_h-=line_height;
						to.g_h+=top_height;

						mt_wind_calc_grect(WC_BORDER,WIND_KIND,&to,&to,&global);
	
						mt_wind_set_grect(window_handle,WF_CURRXYWH,&to,&global);
	
						if(calc_slider())
						{
							int msg[8];
	
							msg[0]=WM_REDRAW;
							msg[1]=ap_id;
							msg[2]=0;
							msg[3]=window_handle;
							mt_wind_get_grect(window_handle,WF_WORKXYWH,(GRECT *)&msg[4],&global);
							mt_appl_write(ap_id,16,msg,&global);
						}
					}
					else
					{
						mt_wind_calc_grect(WC_WORK,WIND_KIND,&full,&work,&global);
						work.g_h=(work.g_h/line_height)*line_height;
	
						lines_to_show=work.g_h/line_height-1;
	
						work.g_h-=line_height;
						work.g_h+=top_height;
	
						mt_wind_calc_grect(WC_BORDER,WIND_KIND,&work,&work,&global);
	
						mt_wind_set_grect(window_handle,WF_CURRXYWH,&work,&global);
	
						if(calc_slider())
						{
							int msg[8];
	
							msg[0]=WM_REDRAW;
							msg[1]=ap_id;
							msg[2]=0;
							msg[3]=window_handle;
							mt_wind_get_grect(window_handle,WF_WORKXYWH,(GRECT *)&msg[4],&global);
							mt_appl_write(ap_id,16,msg,&global);
						}

						full.g_w=full.g_h=-1;
					}
					break;
				}
				case WM_REDRAW:
				{
					if(mt_wind_update(BEG_UPDATE|there_is_check_and_set,&global))
					{
						redraw_window((GRECT *)&events.msg[4]);
						mt_wind_update(END_UPDATE,&global);
					}
					else
					{
						mt_appl_write(ap_id,16,events.msg,&global);
						events.mwhich&=~MU_MESAG;
					}
					break;
				}
				case 0x9277:
				{
					if(events.msg[1]==ap_id &&
						events.msg[3]==(int) 0x9277)
					{
						if(mt_wind_update(BEG_UPDATE|there_is_check_and_set,&global))
						{
							if(events.msg[4]=='cs')
							{
								if(calc_slider())
								{
									GRECT r;

									mt_wind_get_grect(window_handle,WF_WORKXYWH,&r,&global);
	
									redraw_window(&r);
								}
							}
							else if(events.msg[4]=='si')
							{
								set_info_line();
							}
							mt_wind_update(END_UPDATE,&global);
						}
						else
						{
							mt_appl_write(ap_id,16,events.msg,&global);
							events.mwhich&=~MU_MESAG;
						}
					}
					break;
				}
				case WM_SIZED:
				{
					GRECT a;

					full.g_w=full.g_h=-1;

					mt_wind_calc_grect(WC_WORK,WIND_KIND,(GRECT *)&events.msg[4],&a,&global);
					a.g_h=(a.g_h/line_height)*line_height;

					lines_to_show=a.g_h/line_height-1;

					a.g_h-=line_height;
					a.g_h+=top_height;

					mt_wind_calc_grect(WC_BORDER,WIND_KIND,&a,&a,&global);

					mt_wind_set_grect(window_handle,WF_CURRXYWH,&a,&global);

					if(calc_slider())
					{
						int msg[8];

						msg[0]=WM_REDRAW;
						msg[1]=ap_id;
						msg[2]=0;
						msg[3]=window_handle;
						mt_wind_get_grect(window_handle,WF_WORKXYWH,(GRECT *)&msg[4],&global);
						mt_appl_write(ap_id,16,msg,&global);
					}
	
					break;
				}
				case WM_ARROWED:
				{
					GRECT a;

					mt_wind_get_grect(window_handle,WF_WORKXYWH,&a,&global);
					switch(events.msg[4])
					{
						case WA_UPPAGE:
							move_up_down(-lines_to_show+1);
							break;
						case WA_DNPAGE:
							move_up_down(lines_to_show-1);
							break;
						case WA_UPLINE:
							move_up_down(-1);
							break;
						case WA_DNLINE:
							move_up_down(1);
							break;
						case WA_LFPAGE:
							move_left_right(-a.g_w+8);
							break;
						case WA_RTPAGE:
							move_left_right(a.g_w-8);
							break;
						case WA_LFLINE:
							move_left_right(-8);
							break;
						case WA_RTLINE:
							move_left_right(8);
							break;
					}
					break;
				}
				case WM_VSLID:
				{
					move_up_down(((long) ((double) ((double) (((double) events.msg[4])*((double) (finfos-lines_to_show))))/1000))-first_shown);
					break;
				}
				case WM_HSLID:
				{
					GRECT a;

					mt_wind_get_grect(window_handle,WF_WORKXYWH,&a,&global);
				
					a.g_x=6;
					for(a.g_h=0;a.g_h<6;a.g_h++)
					{
						if(show_row[a.g_h])
							a.g_x+=max_breite[a.g_h];
					}

					move_left_right((int) ((long) ((double) ((double) (((double) events.msg[4])*((double) (a.g_x-a.g_w))))/1000))-scrolled_left);
					break;
				}
				case WM_CLOSED:
				case AP_TERM:
					shaded_or_iconified=0;
					events.mwhich|=MU_KEYBD;
					events.kstate=0;
					events.key=CNTRL_Q;
					break;
				case AV_SENDKEY:
					if(events.mwhich & MU_KEYBD)
					{
						mt_appl_write(ap_id,16,events.msg,&global);
					}
					else
					{
						events.mwhich|=MU_KEYBD;
						events.kstate=events.msg[3];
						events.key=events.msg[4];
					}
					break;
				case AV_SENDCLICK:
					if(events.mwhich & MU_BUTTON)
					{
						mt_appl_write(ap_id,16,events.msg,&global);
					}
					else
					{
						events.mwhich|=MU_BUTTON;
						events.mx=events.msg[3];
						events.my=events.msg[4];
						events.mbutton=events.msg[5];
						events.kstate=events.msg[6];
						events.mclicks=events.msg[7];
					}
					break;
				case VA_PROTOSTATUS:
					av_server=mt_appl_find((char *)(((((unsigned long) events.msg[6])<<16) & 0xffff0000l)|(((unsigned long) events.msg[7]) & 0x0000ffffl)),&global);
					if(av_server<0)
						av_server=events.msg[1];
					av_server_kennt=(long) (((((unsigned long) events.msg[3])<<16) & 0xffff0000l)|(((unsigned long) events.msg[4]) & 0x0000ffffl));

					if(av_server_kennt & 0x00020000l)
					{
						int msg[8];

						msg[0]=AV_ASKFILEFONT;
						msg[1]=ap_id;
						msg[2]=0;
						msg[3]=0;
						msg[4]=0;
						msg[5]=0;
						msg[6]=0;
						msg[7]=0;
						mt_appl_write(av_server,16,msg,&global);
					}
					break;
				case VA_FILEFONT:
				case VA_FONTCHANGED:
					set_new_font(events.msg[3],events.msg[4]);
					break;
				case VA_WINDOPEN:
				case VA_XOPEN:
					if(events.msg[3]==0)
					{
						mt_rsrc_gaddr(5,ERR_OPEN_WIND,&alert,&global);
						mt_form_alert(1,alert,&global);
					}
					if ( av_string )
					{
						Mfree(av_string);
						av_string=NULL;
					}
					events.mwhich = 0;
 					break;
				case VA_PROGSTART:
				case VA_VIEWED:
					if(av_string)
					{
						if(events.msg[3]==0)
						{
							mt_rsrc_gaddr(5,ERR_AV_PS_VI,&alert,&global);
							mt_form_alert(1,alert,&global);
						}

						Mfree(av_string);
						av_string=NULL;
						simuliere_taste_button(&events);
						return 0;
					}
					break;
				case VA_THAT_IZIT:
					simuliere_taste_button(&events);
					return 0;
				case VA_DRAG_COMPLETE:
					if(av_string)
					{
						Mfree(av_string);
						av_string=NULL;
						simuliere_taste_button(&events);
						return 0;
					}
					break;
				case VA_FILEDELETED:
					if(events.msg[3])
					{ /* Datei(en) konnte gelîscht werden */
						if(av_del_list)
						{
							GRECT r;
							unsigned long *nr;
							unsigned long zahl;
							FILE_INFO *par;

							nr=av_del_list;

							zahl=*nr++;

							while(zahl--)
							{ /* Datei aus der Liste rausnehmen */
								unsigned long l;

								for(l=0;l<finfos;l++)
								{
									par=get_from_list(l+1);
									if(par && par->read_nr==*nr)
									{
										free_from_list(par);
									}
								}

								nr++;
							}

							free(av_del_list);

							mt_wind_get_grect(window_handle,WF_WORKXYWH,&r,&global);

							init_max_breite();
					
							zahl=1;
							do {
								par=get_from_list(zahl);
								zahl++;
								if(par)
									test_max_breite(par);
							} while(par);
							calc_slider();
							set_info_line();
							redraw_window(&r);
						}
					}
					else
					{ /* Fehler beim Lîschen */
						mt_rsrc_gaddr(5,ERR_DEL_FILE,&alert,&global);
						mt_form_alert(1,alert,&global);
					}
					if(av_string)
					{
						Mfree(av_string);
						av_string=NULL;
						simuliere_taste_button(&events);
						return 0;
					}
					break;
				case VA_FILECHANGED:
					if(av_string)
					{
						Mfree(av_string);
						av_string=(char *) (*(unsigned long *)(&(events.msg[3])));
						simuliere_taste_button(&events);
						return 0;
					}
					break;
				case BUBBLEGEM_ACK:
					if((void *) *((unsigned long *)(&(events.msg[5])))!=NULL)
						Mfree((char *) *(unsigned long *)(&(events.msg[5])));
					break;
				case BUBBLEGEM_REQUEST:
					if(events.msg[6]==0)
						bubble_hilfen(events.msg[3],events.msg[4],events.msg[5]);
					break;
				case AP_DRAGDROP:
				{
				    static char pipename[]="U:\\PIPE\\DRAGDROP.AA";
				    long fd;

				    pipename[18]=events.msg[7] & 0x00ff;
				    pipename[17]=(events.msg[7] & 0xff00) >> 8;

				    fd=Fopen(pipename,2);
				    if(fd>=0)
				    {
				        char c=1;

				        Fwrite((int) fd,1,&c);
				        Fclose((int) fd);
				    }
				}
				break;
				case GS_REQUEST:
				case GS_COMMAND:
				case GS_QUIT:
					if ( gem_script ( events.msg ) )
					{
						shaded_or_iconified=0;
						events.mwhich|=MU_KEYBD;
						events.kstate=0;
						events.key=CNTRL_Q;
					}
					break;
			}
		}

		if(shaded_or_iconified)	/* Keine TastendrÅcke durchlassen */
		{
			events.mwhich&=~MU_KEYBD;
			events.mwhich&=~MU_BUTTON;
		}

		if(events.mwhich & MU_KEYBD)
		{
			GRECT r;

			mt_wind_get_grect(window_handle,WF_WORKXYWH,&r,&global);

			switch(events.key)
			{
				case CNTRL_0:
				case 0x0b1d:	/* Shift-Cntrl-0 */
				case CNTRL_1:
				case 0x0201:	/* Shift-Cntrl-1 */
				case CNTRL_2:
				case 0x0302:	/* Shift-Cntrl-2 */
				case CNTRL_3:
				case 0x041d:	/* Shift-Cntrl-3 */
				case CNTRL_4:
				case 0x0504:	/* Shift-Cntrl-4 */
				case CNTRL_5:
				case 0x0605:	/* Shift-Cntrl-5 */
				case CNTRL_6:
				case 0x0706:	/* Shift-Cntrl-6 */
				{
					int so;
					GRECT r;

					mt_wind_get_grect(window_handle,WF_WORKXYWH,&r,&global);
					so=1;
					switch(events.key)
					{
						case CNTRL_1:
							so=2;
							break;
						case CNTRL_2:
							so=3;
							break;
						case CNTRL_3:
							so=4;
							break;
						case CNTRL_4:
							so=5;
							break;
						case CNTRL_5:
							so=6;
							break;
						case CNTRL_6:
							so=7;
							break;
						case 0x0b1d:	/* Shift-Cntrl-0 */
							so=-1;
							break;
						case 0x0201:	/* Shift-Cntrl-1 */
							so=-2;
							break;
						case 0x0302:	/* Shift-Cntrl-2 */
							so=-3;
							break;
						case 0x041d:	/* Shift-Cntrl-3 */
							so=-4;
							break;
						case 0x0504:	/* Shift-Cntrl-4 */
							so=-5;
							break;
						case 0x0605:	/* Shift-Cntrl-5 */
							so=-6;
							break;
						case 0x0706:	/* Shift-Cntrl-6 */
							so=-7;
							break;
					}

					sortiere_neu(so);

					redraw_window(&r);

					break;
				}
				case CNTRL_I:
				case TAB:
				{
					FILE_INFO *par;
					unsigned long o;
					int not_sendable=FALSE,ret;

					if(av_server_kennt & 0x80000000l)
					{
						for(o=0;o<finfos;o++)
						{
							par=get_from_list(o+1);
							if(par && par->selected)
							{
								if(test_quote(par,NULL,NULL) &&
									!(av_server_kennt & 0x40000000l))
								{
									not_sendable=TRUE;
								}
								else
								{
									av_string=make_quote(par,0);
									if(!av_string)
									{
										mt_rsrc_gaddr(5,ERR_NO_MEM,&alert,&global);
										mt_form_alert(1,alert,&global);
									}
									else
									{
										int msg[8];

										msg[0]=AV_FILEINFO;
										msg[1]=ap_id;
										msg[2]=0;
										*(unsigned long *)(&(msg[3]))=(unsigned long) av_string;
										msg[5]=msg[6]=msg[7]=0;

										mt_appl_write(av_server,16,msg,&global);

										thread_may_exit=0;
										ret=master_event(0);
										thread_may_exit=1;
										if(ret)
											return ret;

										msg[0]=AV_STARTED;
										msg[1]=ap_id;
										msg[2]=0;
										*(unsigned long *)(&(msg[3]))=(unsigned long) av_string;
										msg[5]=msg[6]=msg[7]=0;

										av_string=0l;
									}
								}
							}
						}

						if(not_sendable)
						{
							mt_rsrc_gaddr(5,ERR_HAS_NO_SP,&alert,&global);
							mt_form_alert(1,alert,&global);
						}
					}
					else
					{
						mt_rsrc_gaddr(5,ERR_HAS_NO_SP,&alert,&global);
						mt_form_alert(1,alert,&global);
					}
					break;
				}
				case CNTRL_A:
				{
					unsigned long l;

					for(l=0;l<finfos;l++)
					{
						FILE_INFO *par;

						par=get_from_list(l+1);
						if(par)
							par->selected=TRUE;
					}

					redraw_window(&r);
					break;
				}
				case 0x011b: /* ESC */
					return -2;
				case CNTRL_Q:
				case CNTRL_U:
					return -1;
				case CNTRL_C:
					write_to_scrap ();
				break;
				case CNTRL_F:
					return -3;
				case ALT_O:
					handle_option();
				break;
				case HOME:
					get_from_list(0);
					move_up_down(-finfos);
					break;
				case SHFT_HOME:
					move_up_down(finfos);
					break;
				case SHFT_CU:
				case 0x4900:	/* Page Up */
					move_up_down(-lines_to_show+1);
					break;
				case SHFT_CD:
				case 0x5100:	/* Page Down */
					move_up_down(lines_to_show-1);
					break;
				case CUR_UP:
					move_up_down(-1);
					break;
				case CUR_DOWN:
					move_up_down(1);
					break;
				case SHFT_CL:
					move_left_right(-r.g_w+8);
					break;
				case SHFT_CR:
					move_left_right(r.g_w-8);
					break;
				case CUR_LEFT:
					move_left_right(-8);
					break;
				case CUR_RIGHT:
					move_left_right(8);
					break;
				case HELP:
				{
					int stg_id;

					stg_id=mt_appl_find("ST-GUIDE",&global);
					if(stg_id>=0)
					{
						int buffer[8];

						buffer[0]=VA_START;
						buffer[1]=ap_id;
						buffer[2]=0;
						buffer[3]=(int) ((((long) help_str_liste) & 0xffff0000l)>>16);
						buffer[4]=(int) (((long) help_str_liste) & 0xffff);
						mt_appl_write(stg_id,16,buffer,&global);
					}
					else
					{
						mt_rsrc_gaddr(5,ERR_NO_ST_GUIDE,&alert,&global);
						mt_form_alert(1,alert,&global);
					}
					break;
				}
			}
		}

		if(events.mwhich & MU_BUTTON)
		{
			int ret; 
			
			ret = handle_button ( );
			if (ret)
				return ret;
		}

	} while(events.mwhich!=MU_TIMER);

	return 0;
}
