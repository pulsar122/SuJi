#include "SuJi_glo.h"

extern int doppelklick;

void open_path (char *path, char *name )
{
	int msg[8];

	doppelklick = TRUE;
	
	if(av_server_kennt & 0x08000000l)
	{
		msg[0]=AV_XWIND;
		msg[7]=2;
	}
	else
	{
		msg[0]=AV_OPENWIND;
		msg[7]=0;
	}

	msg[1]=ap_id;
	msg[2]=0;
	msg[3]=(int) ((((long) path) & 0xffff0000l)/0x00010000l);
	msg[4]=(int) (((long) path) & 0x0000ffffl);
	msg[5]=(int) ((((long) name) & 0xffff0000l)/0x00010000l);
	msg[6]=(int) (((long) name) & 0x0000ffffl);
	
	appl_write(av_server,16,msg);
}

int handle_button ( void )
{
	GRECT r;
	int ww;

	ww=6;
	for(i=0;i<6;i++)
	{
		if(show_row[i])
			ww+=max_breite[i];
	}

	mt_wind_get_grect(window_handle,WF_WORKXYWH,&r,&global);

	if(events.mbutton==2 && events.kstate==0) 
	{	/* Klick mit der rechten Maustaste */
		OBJECT *tree;
		char *str=0l;

		mt_rsrc_gaddr(0,BUBBLE_HILFEN,&tree,&global);

		if(events.mx>=r.g_x && events.mx<=r.g_x+r.g_w &&
			events.my>=r.g_y && events.my<=r.g_y+r.g_h)	
		{ /* Im Fenster */
			if(events.mx<=r.g_x+ww)
			{ /* In der Liste */
				if(events.my<=r.g_y+top_height)
				{ /* Im Listenkopf */
					bubble_hilfen(window_handle,events.mx,events.my);
				}
				else
				{ /* Auf eine der Dateien -> Verschieben/Doppelklick */
					EVNTDATA md;

					mt_wind_update(BEG_MCTRL,&global);

					mt_graf_mkstate(&md.x,&md.y,&md.bstate,&md.bstate,&global);

					if(md.bstate==0)
					{ /* Doppelklick */
						events.mbutton=1;
						events.mclicks=2;
					}
					else
					{ /* Verschieben */
						set_mouse(FLAT_HAND);
						do {
							mt_graf_mkstate(&md.x,&md.y,&md.bstate,&md.bstate,&global);
							if(md.y-events.my>=line_height || events.my-md.y>=line_height)
							{
								move_up_down((events.my-md.y)/line_height);
								events.my-=((events.my-md.y)/line_height)*line_height;
							}
							if(md.x!=events.mx)
							{ /* horizontal pixelweise Scroolen */
								move_left_right(events.mx-md.x);
								events.mx=md.x;
							}
						} while(md.bstate==2);
						set_mouse(ARROW);
					}

					mt_wind_update(END_MCTRL,&global);
				}
			}
			else
				bubble_hilfen(window_handle,events.mx,events.my);
		}

		if(str)
			bubble_hilfen(window_handle,(int)((((unsigned long) str) & 0xffff0000l)/0x00010000l),(int) (((unsigned long) str) & 0xffffl));
	}
	else if(events.mbutton==2 && events.kstate==K_ALT)
	{ /* Fenster toppen */
		int top;

		mt_wind_get_int(window_handle,WF_TOP,&top,&global);
		if(top == window_handle)
		{ /* untoppen, wenn mgl. */
			if(mt_appl_getinfo(11,&top,NULL,NULL,NULL,&global) &&
			   (top & 0x0040))
			{
				mt_wind_set_int(window_handle,WF_BOTTOM,0,&global);
			}
		}
		else
			mt_wind_set_int(window_handle,WF_TOP,0,&global);
	}

	if(events.mbutton==1)
	{ /* Klick mit der linken Maustaste */
		if(events.mx>=r.g_x && events.mx<=r.g_x+r.g_w &&
			events.my>=r.g_y && events.my<=r.g_y+r.g_h)
		{ /* Im Fenster */
			if(events.mx<=r.g_x+ww)
			{ /* In der Liste */
				if(events.my<=r.g_y+top_height)
				{ /* Im Listenkopf */
					int xx;
					int i=0;

					xx=events.mx-r.g_x+scrolled_left;

					while(xx>=0)
					{
						if(show_row[i])
							xx-=max_breite[i];
						i++;
					}

					xx=i+1;

					if(events.kstate & K_CTRL)
						xx=1;

					if((events.kstate & (K_RSHIFT|K_LSHIFT)) &&
						((xx!=sort_by && -xx!=sort_by) || sort_by>0))
						xx=-xx;

					sortiere_neu(xx);

					redraw_window(&r);
				}
				else
				{ /* Innerhalb der Dateiliste */
					unsigned long eintrag;
					FILE_INFO *par;
					EVNTDATA md;
	
					mt_graf_mkstate(&md.x,&md.y,&md.bstate,&md.kstate,&global);
	
					eintrag=first_shown+(events.my-r.g_y-top_height)/line_height;
	
					par=get_from_list(eintrag+1);

					if(par)
					{ /* Auf einen Eintrag */
						if((!(events.kstate & 3)) &&	/* keine Shifttaste gedrÅckt? */
							(events.mclicks!=2) &&
							!(md.bstate && events.mclicks==1))
						{
							unsigned long l;
	
							for(l=0;l<finfos;l++)
							{
								FILE_INFO *par;
	
								par=get_from_list(l+1);
	
								if(par->selected)
								{
									if(l!=eintrag)
									{
										par->selected=0;

										if(l>=first_shown && l<=first_shown+lines_to_show)
										{
											GRECT red;

											red=r;
											red.g_y+=((int) (l-first_shown))*line_height+top_height;
											red.g_h=line_height;

											redraw_window(&red);
										}
									}
								}
								else if(l==eintrag)
								{
									par->selected=1;

									if(l>=first_shown && l<=first_shown+lines_to_show)
									{
										GRECT red;

										red=r;
										red.g_y+=((int) (l-first_shown))*line_height+top_height;
										red.g_h=line_height;

										redraw_window(&red);
									}
								}
							}
						}
						else
						{ /* Shifttaste gedrÅckt */
							if(!(md.bstate && events.mclicks==1))
							{
								if(events.mclicks!=2 || !par->selected)
								{
									par->selected=!par->selected;
	
									if(eintrag>=first_shown && eintrag<=first_shown+lines_to_show)
									{
										GRECT red;

										red=r;
										red.g_y+=((int) (eintrag-first_shown))*line_height+top_height;
										red.g_h=line_height;

										redraw_window(&red);
									}
								}
							}
							else
							{
								if(!par->selected)
								{
									if(events.kstate & 3)
									{
										par->selected=TRUE;

										if(eintrag>=first_shown && eintrag<=first_shown+lines_to_show)
										{
											GRECT red;

											red=r;
											red.g_y+=((int) (eintrag-first_shown))*line_height+top_height;
											red.g_h=line_height;

											redraw_window(&red);
										}
									}
									else
									{
										unsigned long l;

										par->selected=TRUE;
										redraw_window(&r);

										for(l=0;l<finfos;l++)
										{
											FILE_INFO *par;

											par=get_from_list(l+1);

											if(l!=eintrag && par && par->selected)
											{
												par->selected=FALSE;
												if(l>=first_shown && l<=first_shown+lines_to_show)
												{
													GRECT red;

													red=r;
													red.g_y+=((int) (l-first_shown))*line_height+top_height;
													red.g_h=line_height;

													redraw_window(&red);
												}
											}
										}
									}
								}
							}
						}
	
						if(md.bstate && events.mclicks==1)
						{	/* D&D */
							unsigned long l;
							int msg[8];
							int ret;
						
							if(av_server_kennt & 0x00000002l)
							{
								GRECT xywh;
								EVNTDATA md;

								set_mouse(FLAT_HAND);
								mt_wind_get_grect(0,WF_WORKXYWH,&xywh,&global);
							
								mt_graf_dragbox((r.g_w<ww ? r.g_w : ww),line_height,r.g_x,r.g_y+((int) (eintrag-first_shown))*line_height+top_height,
																	xywh.g_x,xywh.g_y,xywh.g_h,xywh.g_w,&xywh.g_x,&xywh.g_y,&global);
							
								mt_graf_mkstate(&md.x,&md.y,&md.bstate,&md.bstate,&global);

								if(mt_wind_find(md.x,md.y,&global)!=window_handle)
								{
									set_mouse(BUSYBEE);

									if(!(av_server_kennt & 0x40000000l))
									{
										int not_sendable=0;
										unsigned long size=0l;
								
										for(l=0;l<finfos;l++)
										{
											par=get_from_list(l+1);
								
											if(par && par->selected)
											{
												if(test_quote(par,NULL,NULL))
													not_sendable=1;
												else
													size+=strlen(par->pfad)+strlen(par->name)+1;
											}
										}
								
										if(not_sendable)
										{
											mt_rsrc_gaddr(5,ERR_HAS_NO_QUOTE,&alert,&global);
											mt_form_alert(1,alert,&global);
										}
								
										if(size)
										{
											av_string=(char *)myMxalloc (size+1, 0x22);
								
											if(!av_string)
											{
												mt_rsrc_gaddr(5,ERR_NO_MEM,&alert,&global);
												mt_form_alert(1,alert,&global);
											}
											else
											{
												av_string[0]='\0';
												for(l=0;l<finfos;l++)
												{
													par=get_from_list(l+1);
								
													if(par && par->selected)
													{
														if(!test_quote(par,NULL,NULL))
														{
															strcat(av_string,par->pfad);
															strcat(av_string,par->name);
															strcat(av_string," ");
														}
													}
												}
												av_string[strlen(av_string)-1]='\0';
											}
										}
									}
									else
									{
										av_string=make_quote(NULL,0);
										if(!av_string)
										{
											mt_rsrc_gaddr(5,ERR_NO_MEM,&alert,&global);
											mt_form_alert(1,alert,&global);
										}
									}
								
									if(av_string)
									{
										msg[0]=AV_WHAT_IZIT;
										msg[1]=ap_id;
										msg[2]=0;
										msg[3]=md.x;
										msg[4]=md.y;
										msg[5]=msg[6]=msg[7]=0;
								
										mt_appl_write(av_server,16,msg,&global);
								
										thread_may_exit=0;
										ret=master_event(0);
										thread_may_exit=1;
										if(ret)
											return ret;

											/* VA_THAT_IZIT auswerten */
										if((events.msg[4]==VA_OB_TRASHCAN || events.msg[4]==VA_OB_SHREDDER) &&
											(av_server_kennt & 0x00000002l))
										{ /* Dateien sollen gelîscht werden */
											int really;
											unsigned long selektiert=0;

											msg[0]=AV_DELFILE;
											msg[1]=ap_id;
											msg[2]=0;
											msg[3]=(int) ((((long) av_string) & 0xffff0000l)/0x00010000l);
											msg[4]=(int) (((long) av_string) & 0x0000ffffl);
											msg[5]=0;
											msg[6]=0;
											msg[7]=0;

											for(l=0;l<finfos;l++)
											{
												par=get_from_list(l+1);
									
												if(par && par->selected)
													selektiert++;
											}

											av_del_list=malloc(sizeof(unsigned long)*(selektiert+1));
											if(!av_del_list)
											{ /* Speichermangel */
												mt_rsrc_gaddr(5,ASK_MEM_DEL_FILE,&alert,&global);
												really=mt_form_alert(1,alert,&global) == 1;
											}
											else
											{
												unsigned long *help;

												help=av_del_list;

												*help++=selektiert;

												for(l=0;l<finfos;l++)
												{
													par=get_from_list(l+1);
										
													if(par && par->selected)
													{
														*help++=par->read_nr;
													}
												}
												really=1;
											}

											if(really)
											{
												mt_appl_write(av_server,16,msg,&global);

												thread_may_exit=0;
												ret=master_event(0);
												thread_may_exit=1;
												if(ret)
													return ret;
											}
										}
										else
										{
											msg[0]=AV_DRAG_ON_WINDOW;
											msg[1]=ap_id;
											msg[2]=0;
											msg[3]=md.x;
											msg[4]=md.y;
											msg[5]=md.kstate;
											msg[6]=(int) ((((long) av_string) & 0xffff0000l)/0x00010000l);
											msg[7]=(int) (((long) av_string) & 0x0000ffffl);
									
											mt_appl_write(av_server,16,msg,&global);
								
											msg[0]=AV_STARTED;
											msg[1]=ap_id;
											msg[2]=0;
											msg[3]=events.msg[6];
											msg[4]=events.msg[7];
											msg[5]=msg[6]=msg[7]=0;
								
											mt_appl_write(av_server,16,msg,&global);

											thread_may_exit=1;
											ret=master_event(0);
											thread_may_exit=1;
											if(ret)
												return ret;
										}
									}
								}

								set_mouse(ARROW);
							}
						}
	
						if(events.mclicks==2)
						{ /* Doppelklick */
							unsigned long l;
							int w;
							
							w=0;
							for ( i=BR_FILE; i<=BR_ATTR; i++)
							{
								if(show_row[i])
									w +=max_breite[i];
							}

							if(events.mx<=r.g_x+left_space+w)
							{ /* Datei îffnen */
								if(av_server_kennt & 0x00200004l)
								{
									int not_sendable=0;

									for(l=0;l<finfos;l++)
									{
										FILE_INFO *par;

										par=get_from_list(l+1);

										if(!(events.kstate & 3) &&
											l!=eintrag &&
											par && par->selected)
										{
											par->selected=FALSE;
											if(l>=first_shown && l<=first_shown+lines_to_show)
											{
												GRECT red;

												red=r;
												red.g_y+=((int) (l-first_shown))*line_height+top_height;
												red.g_h=line_height;

												redraw_window(&red);
											}
										}

										if(par && par->selected)
										{
											if ( par->attrib & FA_SUBDIR )
											{
												av_string=(char *)myMxalloc (strlen(par->pfad)+strlen(par->name)+1, 0x22);
												if(av_string)
												{
													
													strcpy ( av_string, par->pfad );
													strcat ( av_string, par->name );
													
													open_path ( av_string, par->name );

												}
											}
											else
											{
												av_string=(char *)myMxalloc (strlen(par->pfad)+strlen(par->name)+1, 0x22);

												if(!av_string)
													not_sendable++;
												else
												{
													int msg[8];
													int ret;

													strcpy(av_string,par->pfad);
													strcat(av_string,par->name);

													if(!(av_server_kennt & 0x00000004l))
														msg[0]=AV_STARTPROG;
													else if(!(av_server_kennt & 0x00200000l))
														msg[0]=AV_VIEW;
													else if(events.kstate & K_ALT)
														msg[0]=AV_VIEW;
													else
														msg[0]=AV_STARTPROG;

													msg[1]=ap_id;
													msg[2]=0;
													msg[3]=(int) ((((long) av_string) & 0xffff0000l)/0x00010000l);
													msg[4]=(int) (((long) av_string) & 0x0000ffffl);
													msg[5]=0;
													msg[6]=0;
													msg[7]=0;

													mt_appl_write(av_server,16,msg,&global);

													thread_may_exit=0;
													ret=master_event(0);
													thread_may_exit=1;
													if(ret)
														return ret;
												}
											}
										}
									}

									if(not_sendable)
									{
										mt_rsrc_gaddr(5,ERR_NO_MEM,&alert,&global);
										mt_form_alert(1,alert,&global);
									}
								}
								else
								{
									mt_rsrc_gaddr(5,ERR_HAS_NO_SP,&alert,&global);
									mt_form_alert(1,alert,&global);
								}
							}
							else
							{ /* Pfad îffnen */
								if(av_server_kennt & 0x08100000l)
								{
									for(l=0;l<finfos;l++)
									{
										FILE_INFO *par;

										par=get_from_list(l+1);

										if(!(events.kstate & 3) &&
											l!=eintrag &&
											par && par->selected)
										{
											par->selected=FALSE;
											if(l>=first_shown && l<=first_shown+lines_to_show)
											{
												GRECT red;

												red=r;
												red.g_y+=((int) (l-first_shown))*line_height+top_height;
												red.g_h=line_height;

												redraw_window(&red);
											}
										}

										if(par && par->selected)
										{
											av_string=(char *)myMxalloc ( strlen(par->name)+1, 0x22);
											if ( av_string )
											{
												long size;
												
												strcpy ( av_string, par->name ); 
												size = strlen ( av_string );
												if ( av_string[size-1] == '\\' )
													av_string [size-1] = '\0';
										
												open_path (par->pfad, av_string );
											}
										}
									}
								}
								else
								{
									mt_rsrc_gaddr(5,ERR_HAS_NO_WO,&alert,&global);
									mt_form_alert(1,alert,&global);
								}
							}
						}
					}
					else
					{ /* auf keinen Eintrag */
						unsigned long l;

						for(l=0;l<finfos;l++)
						{
							FILE_INFO *par;

							par=get_from_list(l+1);

							if(par && par->selected)
							{
								par->selected=FALSE;
								if(l>=first_shown && l<=first_shown+lines_to_show)
								{
									GRECT red;

									red=r;
									red.g_y+=((int) (l-first_shown))*line_height+top_height;
									red.g_h=line_height;

									redraw_window(&red);
								}
							}
						}
					}
				}
			}
			else
			{ /* Neben die Liste */
				unsigned long l;

				for(l=0;l<finfos;l++)
				{
					FILE_INFO *par;

					par=get_from_list(l+1);

					if(par && par->selected)
					{
						par->selected=FALSE;
						if(l>=first_shown && l<=first_shown+lines_to_show)
						{
							GRECT red;

							red=r;
							red.g_y+=((int) (l-first_shown))*line_height+top_height;
							red.g_h=line_height;

							redraw_window(&red);
						}
					}
				}
			}
		}
	}
	
	return FALSE;
}
