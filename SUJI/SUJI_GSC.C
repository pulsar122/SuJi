#include "SuJi_glo.h"

char 			*gslongname;
GS_INFO   *gsi = NULL;
int       gsapp = -1;

static int doGSCommand( int msg[8] );

/* Return: 0 = OK 																												*/
/*				 1 = Command "Quit"																							*/

int gem_script ( int msg[8] )
{
	int answ[8];
	
	switch ( msg[0] )
	{
		case GS_REQUEST:
			answ[0]=GS_REPLY;
			answ[1]=ap_id;
			answ[2]=0;
			answ[3]=0;
			answ[4]=0;
			answ[5]=0;
			answ[6]=1;
			answ[7]=msg[7];
	
			if (!gsi) gsi = (GS_INFO *)myMxalloc(sizeof(GS_INFO), 3|MGLOBAL);
			
			if (gsi)
			{
				GS_INFO *sender = *(GS_INFO **)&msg[3];
	
				gsi->len     = sizeof(GS_INFO);
				gsi->version = 0x0100;
				gsi->msgs    = GSM_COMMAND;
				gsi->ext     = 0L;
				
				answ[3]=(int)(((long)gsi >> 16) & 0x0000ffffL);
				answ[4]=(int)((long)gsi & 0x0000ffffL);
				
				if (sender)
				{
					if (sender->version >= 0x0070)
					{
						answ[6]=0;
						gsapp=msg[1];
					}
				}
			}
			
			appl_write(gsapp,16,answ);
		break;
		case GS_COMMAND:
			if ( doGSCommand(msg) )
				return 1;
		break;
		case GS_QUIT:
			if (msg[1]==gsapp)
				gsapp = -1;
		break;
	}

	return 0;
}

int doGSCommand(int msg[8])
{
	int   answ[8], ret=0;
	char *cmd = *(char **)&msg[3];

	answ[0]=GS_ACK;
	answ[1]=ap_id;
	answ[2]=0;
	answ[3]=msg[3];
	answ[4]=msg[4];
	answ[5]=0;
	answ[6]=0;
	answ[7]=GSACK_ERROR;

	if (cmd)
	{
		answ[7]=GSACK_UNKNOWN;

		if (!stricmp(cmd,"Quit"))
		{
			ret=1;
			answ[7]=GSACK_OK;
		}
		else if (!stricmp(cmd,"AppGetLongName"))
		{
			if (gslongname)
			{
				answ[5]=(int)(((long)gslongname >> 16) & 0x0000ffffL);
				answ[6]=(int)((long)gslongname & 0x0000ffffL);
				answ[7]=GSACK_OK;
			}
			else
			{
				answ[7]=GSACK_ERROR;
			}
		}
		else if (!stricmp(cmd,"ToFront"))
		{
			if ( window_handle != -1 )
				mt_wind_set(window_handle,WF_TOP,0,0,0,0,&global);
			else
				mt_wind_set(dialog_maske.w_handle,WF_TOP,0,0,0,0,&global);
			answ[7]=GSACK_OK;
		}
		else if (!stricmp(cmd,"Copy"))
		{
			if ( window_handle != -1 )
			{
				write_to_scrap ();
				answ[7]=GSACK_OK;
			}
		}
	}

	appl_write(msg[1],16,answ);

	return(ret);
}

void init_gem_script ( void )
{
	gslongname = (char *)myMxalloc(256, 3|MGLOBAL );
	if ( gslongname )
		strcpy ( gslongname, "SuJi" );
}

void exit_gem_script ( void )
{
	if ( gslongname )
		Mfree ( gslongname );
}