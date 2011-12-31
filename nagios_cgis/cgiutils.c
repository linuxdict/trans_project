/***********************************************************************
 *
 * CGIUTILS.C - Common utilities for Nagios CGIs
 * 
 * Copyright (c) 2010 Nagios Core Development Team and Community Contributors
 * Copyright (c) 1999-2010 Ethan Galstad (egalstad@nagios.org)
 * Last Modified: 08-05-2010
 *
 * License:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ***********************************************************************/

#include "../include/config.h"
#include "../include/common.h"
#include "../include/locations.h"
#include "../include/objects.h"
#include "../include/statusdata.h"

#include "../include/cgiutils.h"

char            main_config_file[MAX_FILENAME_LENGTH];
char            log_file[MAX_FILENAME_LENGTH];
char            log_archive_path[MAX_FILENAME_LENGTH];
char            command_file[MAX_FILENAME_LENGTH];

char            physical_html_path[MAX_FILENAME_LENGTH];
char            physical_images_path[MAX_FILENAME_LENGTH];
char            physical_ssi_path[MAX_FILENAME_LENGTH];
char            url_html_path[MAX_FILENAME_LENGTH];
char            url_docs_path[MAX_FILENAME_LENGTH];
char            url_context_help_path[MAX_FILENAME_LENGTH];
char            url_images_path[MAX_FILENAME_LENGTH];
char            url_logo_images_path[MAX_FILENAME_LENGTH];
char            url_stylesheets_path[MAX_FILENAME_LENGTH];
char            url_media_path[MAX_FILENAME_LENGTH];

char            *service_critical_sound=NULL;
char            *service_warning_sound=NULL;
char            *service_unknown_sound=NULL;
char            *host_down_sound=NULL;
char            *host_unreachable_sound=NULL;
char            *normal_sound=NULL;
char            *statusmap_background_image=NULL;
char            *statuswrl_include=NULL;

char            *illegal_output_chars=NULL;

char            *notes_url_target=NULL;
char            *action_url_target=NULL;

char            *ping_syntax=NULL;

char            nagios_check_command[MAX_INPUT_BUFFER]="";
char            nagios_process_info[MAX_INPUT_BUFFER]="";
int             nagios_process_state=STATE_OK;

int             enable_splunk_integration=FALSE;
char            *splunk_url=NULL;
int             lock_author_names=TRUE;

extern time_t   program_start;
extern int      nagios_pid;
extern int      daemon_mode;
extern int      enable_notifications;
extern int      execute_service_checks;
extern int      accept_passive_service_checks;
extern int      enable_event_handlers;
extern int      obsess_over_services;
extern int      enable_failure_prediction;
extern int      process_performance_data;
extern time_t   last_command_check;
extern time_t   last_log_rotation;

int             check_external_commands=0;

int             date_format=DATE_FORMAT_US;

int             log_rotation_method=LOG_ROTATION_NONE;

time_t          this_scheduled_log_rotation=0L;
time_t          last_scheduled_log_rotation=0L;
time_t          next_scheduled_log_rotation=0L;

int             use_authentication=TRUE;

int             interval_length=60;

int             show_context_help=FALSE;

int             use_pending_states=TRUE;

int             host_status_has_been_read=FALSE;
int             service_status_has_been_read=FALSE;
int             program_status_has_been_read=FALSE;

int             refresh_rate=DEFAULT_REFRESH_RATE;

int             escape_html_tags=FALSE;

int             use_ssl_authentication=FALSE;

int             default_statusmap_layout_method=0;
int             default_statuswrl_layout_method=0;

int		color_transparency_index_r=255;
int		color_transparency_index_g=255;
int		color_transparency_index_b=255;

extern hostgroup       *hostgroup_list;
extern contactgroup    *contactgroup_list;
extern command         *command_list;
extern timeperiod      *timeperiod_list;
extern contact         *contact_list;
extern serviceescalation *serviceescalation_list;

extern hoststatus      *hoststatus_list;
extern servicestatus   *servicestatus_list;

lifo            *lifo_list=NULL;

char            *my_strtok_buffer=NULL;
char            *original_my_strtok_buffer=NULL;

char encoded_url_string[2][MAX_INPUT_BUFFER]; // 2 to be able use url_encode twice
char *encoded_html_string=NULL;

#ifdef HAVE_TZNAME
#ifdef CYGWIN
extern char     *_tzname[2] __declspec(dllimport);
#else
extern char     *tzname[2];
#endif
#endif

// add Chinese support for the map/image
char            ttf_file[MAX_FILENAME_LENGTH];


/**********************************************************
 ***************** CLEANUP FUNCTIONS **********************
 **********************************************************/

/* reset all variables used by the CGIs */
void reset_cgi_vars(void){

	strcpy(main_config_file,"");

	strcpy(physical_html_path,"");
	strcpy(physical_images_path,"");
	strcpy(physical_ssi_path,"");

	strcpy(url_html_path,"");
	strcpy(url_docs_path,"");
	strcpy(url_context_help_path,"");
	strcpy(url_stylesheets_path,"");
	strcpy(url_media_path,"");
	strcpy(url_images_path,"");

	strcpy(log_file,"");
	strcpy(log_archive_path,DEFAULT_LOG_ARCHIVE_PATH);
	if(log_archive_path[strlen(log_archive_path)-1]!='/' && strlen(log_archive_path)<sizeof(log_archive_path)-2)
		strcat(log_archive_path,"/");
	strcpy(command_file,get_cmd_file_location());

	strcpy(nagios_check_command,"");
	strcpy(nagios_process_info,"");
	nagios_process_state=STATE_OK;

	log_rotation_method=LOG_ROTATION_NONE;

	use_authentication=TRUE;

	interval_length=60;

	refresh_rate=DEFAULT_REFRESH_RATE;

	default_statusmap_layout_method=0;
	default_statusmap_layout_method=0;

	service_critical_sound=NULL;
	service_warning_sound=NULL;
	service_unknown_sound=NULL;
	host_down_sound=NULL;
	host_unreachable_sound=NULL;
	normal_sound=NULL;

	statusmap_background_image=NULL;
	color_transparency_index_r=255;
	color_transparency_index_g=255;
	color_transparency_index_b=255;
	statuswrl_include=NULL;

	ping_syntax=NULL;

	my_strtok_buffer=NULL;
	original_my_strtok_buffer=NULL;

	strcpy(ttf_file,"/usr/local/nagios/share/simhei.ttf");

	return;
        }



/* free all memory for object definitions */
void free_memory(void){

	/* free memory for common object definitions */
	free_object_data();

	/* free memory for status data */
	free_status_data();

	/* free misc data */
	free(service_critical_sound);
	free(service_warning_sound);
	free(service_unknown_sound);
	free(host_down_sound);
	free(host_unreachable_sound);
	free(normal_sound);
	free(statusmap_background_image);
	free(statuswrl_include);
	free(ping_syntax);

	return;
        }




/**********************************************************
 *************** CONFIG FILE FUNCTIONS ********************
 **********************************************************/

/* read the CGI config file location from an environment variable */
char * get_cgi_config_location(void){
        static char *cgiloc=NULL;

        if(!cgiloc){
                cgiloc=getenv("NAGIOS_CGI_CONFIG");
                if(!cgiloc)
                        cgiloc=DEFAULT_CGI_CONFIG_FILE;
                }

        return cgiloc;
        }


/* read the command file location from an environment variable */
char * get_cmd_file_location(void){
        static char *cmdloc=NULL;

        if(!cmdloc){
                cmdloc=getenv("NAGIOS_COMMAND_FILE");
                if(!cmdloc)
                        cmdloc=DEFAULT_COMMAND_FILE;
                }
        return cmdloc;
        }


/*read the CGI configuration file */
int read_cgi_config_file(char *filename){
	char *input=NULL;
	mmapfile *thefile;
	char *var=NULL;
	char *val=NULL;


	if((thefile=mmap_fopen(filename))==NULL)
		return ERROR;

	while(1){

		/* free memory */
		free(input);

		/* read the next line */
		if((input=mmap_fgets_multiline(thefile))==NULL)
			break;

		strip(input);

		var=strtok(input,"=");
		val=strtok(NULL,"\n");

		if(var==NULL || val==NULL)
			continue;

		if(!strcmp(var,"main_config_file")){
			strncpy(main_config_file,val,sizeof(main_config_file));
			main_config_file[sizeof(main_config_file)-1]='\x0';
			strip(main_config_file);
		        }

		else if(!strcmp(var,"show_context_help"))
			show_context_help=(atoi(val)>0)?TRUE:FALSE;

		else if(!strcmp(var,"use_pending_states"))
			use_pending_states=(atoi(val)>0)?TRUE:FALSE;

		else if(!strcmp(var,"use_authentication"))
			use_authentication=(atoi(val)>0)?TRUE:FALSE;

		else if(!strcmp(var,"nagios_check_command")){
			strncpy(nagios_check_command,val,sizeof(nagios_check_command));
			nagios_check_command[sizeof(nagios_check_command)-1]='\x0';
			strip(nagios_check_command);
		        }

		else if(!strcmp(var,"refresh_rate"))
			refresh_rate=atoi(val);

		else if(!strcmp(var,"physical_html_path")){
			strncpy(physical_html_path,val,sizeof(physical_html_path));
			physical_html_path[sizeof(physical_html_path)-1]='\x0';
			strip(physical_html_path);
			if(physical_html_path[strlen(physical_html_path)-1]!='/' && (strlen(physical_html_path) < sizeof(physical_html_path)-1))
				strcat(physical_html_path,"/");

			snprintf(physical_images_path,sizeof(physical_images_path),"%simages/",physical_html_path);
			physical_images_path[sizeof(physical_images_path)-1]='\x0';

			snprintf(physical_ssi_path,sizeof(physical_images_path),"%sssi/",physical_html_path);
			physical_ssi_path[sizeof(physical_ssi_path)-1]='\x0';
		        }

		else if(!strcmp(var,"url_html_path")){

			strncpy(url_html_path,val,sizeof(url_html_path));
			url_html_path[sizeof(url_html_path)-1]='\x0';

			strip(url_html_path);
			if(url_html_path[strlen(url_html_path)-1]!='/' && (strlen(url_html_path) < sizeof(url_html_path)-1))
				strcat(url_html_path,"/");

			snprintf(url_docs_path,sizeof(url_docs_path),"%sdocs/",url_html_path);
			url_docs_path[sizeof(url_docs_path)-1]='\x0';

			snprintf(url_context_help_path,sizeof(url_context_help_path),"%scontexthelp/",url_html_path);
			url_context_help_path[sizeof(url_context_help_path)-1]='\x0';

			snprintf(url_images_path,sizeof(url_images_path),"%simages/",url_html_path);
			url_images_path[sizeof(url_images_path)-1]='\x0';

			snprintf(url_logo_images_path,sizeof(url_logo_images_path),"%slogos/",url_images_path);
			url_logo_images_path[sizeof(url_logo_images_path)-1]='\x0';

			snprintf(url_stylesheets_path,sizeof(url_stylesheets_path),"%sstylesheets/",url_html_path);
			url_stylesheets_path[sizeof(url_stylesheets_path)-1]='\x0';

			snprintf(url_media_path,sizeof(url_media_path),"%smedia/",url_html_path);
			url_media_path[sizeof(url_media_path)-1]='\x0';
		        }

		else if(!strcmp(var,"service_critical_sound"))
			service_critical_sound=strdup(val);

		else if(!strcmp(var,"service_warning_sound"))
			service_warning_sound=strdup(val);

		else if(!strcmp(var,"service_unknown_sound"))
			service_unknown_sound=strdup(val);

		else if(!strcmp(var,"host_down_sound"))
			host_down_sound=strdup(val);

		else if(!strcmp(var,"host_unreachable_sound"))
			host_unreachable_sound=strdup(val);

		else if(!strcmp(var,"normal_sound"))
			normal_sound=strdup(val);

		else if(!strcmp(var,"statusmap_background_image"))
			statusmap_background_image=strdup(val);

		else if(!strcmp(var,"color_transparency_index_r"))
			color_transparency_index_r=atoi(val);
                
		else if(!strcmp(var,"color_transparency_index_g"))
			color_transparency_index_g=atoi(val);

		else if(!strcmp(var,"color_transparency_index_b"))
			color_transparency_index_b=atoi(val);

		else if(!strcmp(var,"default_statusmap_layout"))
			default_statusmap_layout_method=atoi(val);

		else if(!strcmp(var,"default_statuswrl_layout"))
			default_statuswrl_layout_method=atoi(val);

		else if(!strcmp(var,"statuswrl_include"))
			statuswrl_include=strdup(val);

		else if(!strcmp(var,"ping_syntax"))
			ping_syntax=strdup(val);

		else if(!strcmp(var,"action_url_target"))
			action_url_target=strdup(val);

		else if(!strcmp(var,"illegal_macro_output_chars"))
			illegal_output_chars=strdup(val);

		else if(!strcmp(var,"notes_url_target"))
			notes_url_target=strdup(val);

		else if(!strcmp(var,"enable_splunk_integration"))
			enable_splunk_integration=(atoi(val)>0)?TRUE:FALSE;

		else if(!strcmp(var,"splunk_url"))
			splunk_url=strdup(val);

		else if(!strcmp(var,"escape_html_tags"))
			escape_html_tags=(atoi(val)>0)?TRUE:FALSE;

		else if(!strcmp(var,"lock_author_names"))
			lock_author_names=(atoi(val)>0)?TRUE:FALSE;

		else if(!strcmp(var,"use_ssl_authentication"))
			use_ssl_authentication=(atoi(val)>0)?TRUE:FALSE;
 	        }

	/* free memory and close the file */
	free(input);
	mmap_fclose(thefile);

	if(!strcmp(main_config_file,""))
		return ERROR;
	else
		return OK;
        }



/* read the main configuration file */
int read_main_config_file(char *filename){
	char *input=NULL;
	char *temp_buffer;
	mmapfile *thefile;

	
	if((thefile=mmap_fopen(filename))==NULL)
		return ERROR;

	while(1){

		/* free memory */
		free(input);

		/* read the next line */
		if((input=mmap_fgets_multiline(thefile))==NULL)
			break;

		strip(input);

		if(strstr(input,"interval_length=")==input){
			temp_buffer=strtok(input,"=");
			temp_buffer=strtok(NULL,"\x0");
			interval_length=(temp_buffer==NULL)?60:atoi(temp_buffer);
		        }

		else if(strstr(input,"log_file=")==input){
			temp_buffer=strtok(input,"=");
			temp_buffer=strtok(NULL,"\x0");
			strncpy(log_file,(temp_buffer==NULL)?"":temp_buffer,sizeof(log_file));
			log_file[sizeof(log_file)-1]='\x0';
			strip(log_file);
		        }

		else if(strstr(input,"log_archive_path=")==input){
			temp_buffer=strtok(input,"=");
			temp_buffer=strtok(NULL,"\n");
			strncpy(log_archive_path,(temp_buffer==NULL)?"":temp_buffer,sizeof(log_archive_path));
			log_archive_path[sizeof(log_archive_path)-1]='\x0';
			strip(physical_html_path);
			if(log_archive_path[strlen(log_archive_path)-1]!='/' && (strlen(log_archive_path) < sizeof(log_archive_path)-1))
				strcat(log_archive_path,"/");
		        }

		else if(strstr(input,"log_rotation_method=")==input){
			temp_buffer=strtok(input,"=");
			temp_buffer=strtok(NULL,"\x0");
			if(temp_buffer==NULL)
				log_rotation_method=LOG_ROTATION_NONE;
			else if(!strcmp(temp_buffer,"h"))
				log_rotation_method=LOG_ROTATION_HOURLY;
			else if(!strcmp(temp_buffer,"d"))
				log_rotation_method=LOG_ROTATION_DAILY;
			else if(!strcmp(temp_buffer,"w"))
				log_rotation_method=LOG_ROTATION_WEEKLY;
			else if(!strcmp(temp_buffer,"m"))
				log_rotation_method=LOG_ROTATION_MONTHLY;
		        }

		else if(strstr(input,"command_file=")==input){
			temp_buffer=strtok(input,"=");
			temp_buffer=strtok(NULL,"\x0");
			strncpy(command_file,(temp_buffer==NULL)?"":temp_buffer,sizeof(command_file));
			command_file[sizeof(command_file)-1]='\x0';
			strip(command_file);
		        }

		else if(strstr(input,"check_external_commands=")==input){
			temp_buffer=strtok(input,"=");
			temp_buffer=strtok(NULL,"\x0");
			check_external_commands=(temp_buffer==NULL)?0:atoi(temp_buffer);
		        }

		else if(strstr(input,"date_format=")==input){
			temp_buffer=strtok(input,"=");
			temp_buffer=strtok(NULL,"\x0");
			if(temp_buffer==NULL)
				date_format=DATE_FORMAT_US;
			else if(!strcmp(temp_buffer,"euro"))
				date_format=DATE_FORMAT_EURO;
			else if(!strcmp(temp_buffer,"iso8601"))
				date_format=DATE_FORMAT_ISO8601;
			else if(!strcmp(temp_buffer,"strict-iso8601"))
				date_format=DATE_FORMAT_STRICT_ISO8601;
			else
				date_format=DATE_FORMAT_US;
		        }

		else if(strstr(input,"ttf_file=")==input){
			temp_buffer=strtok(input,"=");
			temp_buffer=strtok(NULL,"\x0");
                        strncpy(ttf_file,(temp_buffer==NULL)?"":temp_buffer,sizeof(ttf_file));
                        ttf_file[sizeof(ttf_file)-1]='\x0';
                        strip(ttf_file);
                        }

               }

	/* free memory and close the file */
	free(input);
	mmap_fclose(thefile);

	return OK;
        }



/* read all object definitions */
int read_all_object_configuration_data(char *config_file,int options){
	int result=OK;

	/* read in all external config data of the desired type(s) */
	result=read_object_config_data(config_file,options,FALSE,FALSE);

	return result;
        }


/* read all status data */
int read_all_status_data(char *config_file,int options){
	int result=OK;

	/* don't duplicate things we've already read in */
	if(program_status_has_been_read==TRUE && (options & READ_PROGRAM_STATUS))
		options-=READ_PROGRAM_STATUS;
	if(host_status_has_been_read==TRUE && (options & READ_HOST_STATUS))
		options-=READ_HOST_STATUS;
	if(service_status_has_been_read==TRUE && (options & READ_SERVICE_STATUS))
		options-=READ_SERVICE_STATUS;

	/* bail out if we've already read what we need */
	if(options<=0)
		return OK;

	/* read in all external status data */
	result=read_status_data(config_file,options);

	/* mark what items we've read in... */
	if(options & READ_PROGRAM_STATUS)
		program_status_has_been_read=TRUE;
	if(options & READ_HOST_STATUS)
		host_status_has_been_read=TRUE;
	if(options & READ_SERVICE_STATUS)
		service_status_has_been_read=TRUE;

	return result;
        }



/**********************************************************
 *************** CHAINED HASH FUNCTIONS *******************
 **********************************************************/


/* dual hash function */
int hashfunc(const char *name1,const char *name2,int hashslots){
	unsigned int i,result;

	result=0;

	if(name1)
		for(i=0;i<strlen(name1);i++)
			result+=name1[i];

	if(name2)
		for(i=0;i<strlen(name2);i++)
			result+=name2[i];

	result=result%hashslots;

	return result;
        }


/* dual hash data comparison */
int compare_hashdata(const char *val1a, const char *val1b, const char *val2a, const char *val2b){
	int result=0;

	/* NOTE: If hash calculation changes, update the compare_strings() function! */

	/* check first name */
	if(val1a==NULL && val2a==NULL)
		result=0;
	else if(val1a==NULL)
		result=1;
	else if(val2a==NULL)
		result=-1;
	else
		result=strcmp(val1a,val2a);

	/* check second name if necessary */
	if(result==0){
		if(val1b==NULL && val2b==NULL)
			result=0;
		else if(val1b==NULL)
			result=1;
		else if(val2b==NULL)
			result=-1;
		else
			result=strcmp(val1b,val2b);
	        }

	return result;
        }




/**********************************************************
 ******************* LIFO FUNCTIONS ***********************
 **********************************************************/

/* reads contents of file into the lifo struct */
int read_file_into_lifo(char *filename){
	char *input=NULL;
	mmapfile *thefile;
	int lifo_result;

	if((thefile=mmap_fopen(filename))==NULL)
		return LIFO_ERROR_FILE;

	while(1){

		free(input);

		if((input=mmap_fgets(thefile))==NULL)
			break;

		lifo_result=push_lifo(input);

		if(lifo_result!=LIFO_OK){
			free_lifo_memory();
			free(input);
			mmap_fclose(thefile);
			return lifo_result;
		        }
	        }

	mmap_fclose(thefile);

	return LIFO_OK;
        }


/* frees all memory allocated to lifo */
void free_lifo_memory(void){
	lifo *temp_lifo;
	lifo *next_lifo;

	if(lifo_list==NULL)
		return;

	temp_lifo=lifo_list;
	while(temp_lifo!=NULL){
		next_lifo=temp_lifo->next;
		if(temp_lifo->data!=NULL)
			free((void *)temp_lifo->data);
		free((void *)temp_lifo);
		temp_lifo=next_lifo;
	        }

	return;
        }


/* adds an item to lifo */
int push_lifo(char *buffer){
	lifo *temp_lifo;

	temp_lifo=(lifo *)malloc(sizeof(lifo));
	if(temp_lifo==NULL)
		return LIFO_ERROR_MEMORY;

	if(buffer==NULL)
		temp_lifo->data=(char *)strdup("");
	else
		temp_lifo->data=(char *)strdup(buffer);
	if(temp_lifo->data==NULL){
		free(temp_lifo);
		return LIFO_ERROR_MEMORY;
	        }

	/* add item to front of lifo... */
	temp_lifo->next=lifo_list;
	lifo_list=temp_lifo;

	return LIFO_OK;
        }



/* returns/clears an item from lifo */
char *pop_lifo(void){
	lifo *temp_lifo;
	char *buf;

	if(lifo_list==NULL || lifo_list->data==NULL)
		return NULL;

	buf=strdup(lifo_list->data);

	temp_lifo=lifo_list->next;

	if(lifo_list->data!=NULL)
		free((void *)lifo_list->data);
	free((void *)lifo_list);

	lifo_list=temp_lifo;

	return buf;
        }




/**********************************************************
 *************** MISC UTILITY FUNCTIONS *******************
 **********************************************************/


/* strip newline, carriage return, and tab characters from beginning and end of a string */
void strip(char *buffer){
	register int x;
	register int y;
	register int z;

	if(buffer==NULL || buffer[0]=='\x0')
		return;

	/* strip end of string */
	y=(int)strlen(buffer);
	for(x=y-1;x>=0;x--){
		if(buffer[x]==' ' || buffer[x]=='\n' || buffer[x]=='\r' || buffer[x]=='\t' || buffer[x]==13)
			buffer[x]='\x0';
		else
			break;
	        }

	/* strip beginning of string (by shifting) */
	y=(int)strlen(buffer);
	for(x=0;x<y;x++){
		if(buffer[x]==' ' || buffer[x]=='\n' || buffer[x]=='\r' || buffer[x]=='\t' || buffer[x]==13)
			continue;
		else
			break;
	        }
	if(x>0){
		for(z=x;z<y;z++)
			buffer[z-x]=buffer[z];
		buffer[y-x]='\x0';
	        }

	return;
	}


/* unescapes newlines in a string */
char *unescape_newlines(char *rawbuf){
	register int x,y;

	for(x=0,y=0;rawbuf[x]!=(char)'\x0';x++){

		if(rawbuf[x]=='\\'){

			/* unescape newlines */
			if(rawbuf[x+1]=='n'){
				rawbuf[y++]='\n';
				x++;
				}

			/* unescape backslashes and other stuff */
			if(rawbuf[x+1]!='\x0'){
				rawbuf[y++]=rawbuf[x+1];
				x++;
				}

			}
		else
			rawbuf[y++]=rawbuf[x];
		}
	rawbuf[y]='\x0';

	return rawbuf;
	}


/* strips HTML and bad stuff from plugin output */
void sanitize_plugin_output(char *buffer){
	int x=0;
	int y=0;
	int in_html=FALSE;
	char *new_buffer;

	if(buffer==NULL)
		return;

	new_buffer=strdup(buffer);
	if(new_buffer==NULL)
		return;

	/* check each character */
	for(x=0,y=0;buffer[x]!='\x0';x++){

		/* we just started an HTML tag */
		if(buffer[x]=='<'){
			in_html=TRUE;
			continue;
		        }

		/* end of an HTML tag */
		else if(buffer[x]=='>'){
			in_html=FALSE;
			continue;
		        }

		/* skip everything inside HTML tags */
		else if(in_html==TRUE)
			continue;

		/* strip single and double quotes */
		else if(buffer[x]=='\'' || buffer[x]=='\"')
			new_buffer[y++]=' ';

		/* strip semicolons (replace with colons) */
		else if(buffer[x]==';')
			new_buffer[y++]=':';

		/* strip pipe and ampersand */
		else if(buffer[x]=='&' || buffer[x]=='|')
			new_buffer[y++]=' ';

		/* normal character */
		else
			new_buffer[y++]=buffer[x];
	        }

	/* terminate sanitized buffer */
	new_buffer[y++]='\x0';

	/* copy the sanitized buffer back to the original */
	strcpy(buffer,new_buffer);

	/* free memory allocated to the new buffer */
	free(new_buffer);

	return;
        }



/* get date/time string */
void get_time_string(time_t *raw_time,char *buffer,int buffer_length,int type){
	time_t t;
	struct tm *tm_ptr=NULL;
	int hour=0;
	int minute=0;
	int second=0;
	int month=0;
	int day=0;
	int year=0;
	char *weekdays[7]={"周日","周一","周二","周三","周四","周五","周六"};
	char *months[12]={"1月","2月","3月","4月","5月","6月","7月","8月","9月","10月","11月","12月"};
	char *tzone="";

	if(raw_time==NULL)
		time(&t);
	else 
		t=*raw_time;

	if(type==HTTP_DATE_TIME)
		tm_ptr=gmtime(&t);
	else
		tm_ptr=localtime(&t);

	hour=tm_ptr->tm_hour;
	minute=tm_ptr->tm_min;
	second=tm_ptr->tm_sec;
	month=tm_ptr->tm_mon+1;
	day=tm_ptr->tm_mday;
	year=tm_ptr->tm_year+1900;

#ifdef HAVE_TM_ZONE
	tzone=(char *)tm_ptr->tm_zone;
#else
	tzone=(tm_ptr->tm_isdst)?tzname[1]:tzname[0];
#endif

	/* ctime() style */
	if(type==LONG_DATE_TIME)
		snprintf(buffer,buffer_length,"%s %s %d %02d:%02d:%02d %s %d",weekdays[tm_ptr->tm_wday],months[tm_ptr->tm_mon],day,hour,minute,second,tzone,year);

	/* short style */
	else if(type==SHORT_DATE_TIME){
		if(date_format==DATE_FORMAT_EURO)
			snprintf(buffer,buffer_length,"%02d-%02d-%04d %02d:%02d:%02d",tm_ptr->tm_mday,tm_ptr->tm_mon+1,tm_ptr->tm_year+1900,tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec);
		else if(date_format==DATE_FORMAT_ISO8601 || date_format==DATE_FORMAT_STRICT_ISO8601)
			snprintf(buffer,buffer_length,"%04d-%02d-%02d%c%02d:%02d:%02d",tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday,(date_format==DATE_FORMAT_STRICT_ISO8601)?'T':' ',tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec);
		else
			snprintf(buffer,buffer_length,"%02d-%02d-%04d %02d:%02d:%02d",tm_ptr->tm_mon+1,tm_ptr->tm_mday,tm_ptr->tm_year+1900,tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec);
	        }

	/* short date */
	else if(type==SHORT_DATE){
		if(date_format==DATE_FORMAT_EURO)
			snprintf(buffer,buffer_length,"%02d-%02d-%04d",day,month,year);
		else if(date_format==DATE_FORMAT_ISO8601 || date_format==DATE_FORMAT_STRICT_ISO8601)
			snprintf(buffer,buffer_length,"%04d-%02d-%02d",year,month,day);
		else
			snprintf(buffer,buffer_length,"%02d-%02d-%04d",month,day,year);
	        }

	/* expiration date/time for HTTP headers */
	else if(type==HTTP_DATE_TIME)
		snprintf(buffer,buffer_length,"%s, %02d %s %d %02d:%02d:%02d GMT",weekdays[tm_ptr->tm_wday],day,months[tm_ptr->tm_mon],year,hour,minute,second);

	/* short time */
	else
		snprintf(buffer,buffer_length,"%02d:%02d:%02d",hour,minute,second);

	buffer[buffer_length-1]='\x0';

	return;
        }



/* given a date/time in time_t format, produce a corresponding date/time string, including timezone */
void get_datetime_string(time_t *raw_time,char *buffer,int buffer_length, int type){
	time_t t;
	struct tm *tm_ptr=NULL;
	int hour=0;
	int minute=0;
	int second=0;
	int month=0;
	int day=0;
	int year=0;
	char *weekdays[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
	char *months[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sept","Oct","Nov","Dec"};

#ifdef HAVE_TM_ZONE
# define tzone tm_ptr->tm_zone
#else
# define tzone (tm_ptr->tm_isdst)?tzname[1]:tzname[0]
#endif

	if(raw_time==NULL)
		time(&t);
	else 
		t=*raw_time;

	tm_ptr=localtime(&t);

	hour=tm_ptr->tm_hour;
	minute=tm_ptr->tm_min;
	second=tm_ptr->tm_sec;
	month=tm_ptr->tm_mon+1;
	day=tm_ptr->tm_mday;
	year=tm_ptr->tm_year+1900;

	/* ctime() style date/time */
	if(type==LONG_DATE_TIME)
		snprintf(buffer,buffer_length,"%s %s %d %02d:%02d:%02d %s %d",weekdays[tm_ptr->tm_wday],months[tm_ptr->tm_mon],day,hour,minute,second,tzone,year);

	/* short date/time */
	else if(type==SHORT_DATE_TIME){
		if(date_format==DATE_FORMAT_EURO)
			snprintf(buffer,buffer_length,"%02d-%02d-%04d %02d:%02d:%02d",day,month,year,hour,minute,second);
		else if(date_format==DATE_FORMAT_ISO8601 || date_format==DATE_FORMAT_STRICT_ISO8601)
			snprintf(buffer,buffer_length,"%04d-%02d-%02d%c%02d:%02d:%02d",year,month,day,(date_format==DATE_FORMAT_STRICT_ISO8601)?'T':' ',hour,minute,second);
		else
			snprintf(buffer,buffer_length,"%02d-%02d-%04d %02d:%02d:%02d",month,day,year,hour,minute,second);
	        }

	/* short date */
	else if(type==SHORT_DATE){
		if(date_format==DATE_FORMAT_EURO)
			snprintf(buffer,buffer_length,"%02d-%02d-%04d",day,month,year);
		else if(date_format==DATE_FORMAT_ISO8601 || date_format==DATE_FORMAT_STRICT_ISO8601)
			snprintf(buffer,buffer_length,"%04d-%02d-%02d",year,month,day);
		else
			snprintf(buffer,buffer_length,"%02d-%02d-%04d",month,day,year);
	        }

	/* short time */
	else
		snprintf(buffer,buffer_length,"%02d:%02d:%02d",hour,minute,second);

	buffer[buffer_length-1]='\x0';

	/* don't mess up other functions that might want to call a variable 'tzone' */
#undef tzone

	return;
        }


/* get time string for an interval of time */
void get_interval_time_string(double time_units,char *buffer,int buffer_length){
	unsigned long total_seconds;
	int hours=0;
	int minutes=0;
	int seconds=0;

	total_seconds=(unsigned long)(time_units*interval_length);
	hours=(int)total_seconds/3600;
	total_seconds%=3600;
	minutes=(int)total_seconds/60;
	total_seconds%=60;
	seconds=(int)total_seconds;
	snprintf(buffer,buffer_length,"%dh %dm %ds",hours,minutes,seconds);
	buffer[buffer_length-1]='\x0';

	return;
        }


/* fix the problem with strtok() skipping empty options between tokens */	
char *my_strtok(char *buffer,char *tokens){
	char *token_position;
	char *sequence_head;

	if(buffer!=NULL){
		if(original_my_strtok_buffer!=NULL)
			free(original_my_strtok_buffer);
		my_strtok_buffer=malloc(strlen(buffer)+1);
		if(my_strtok_buffer==NULL)
			return NULL;
		original_my_strtok_buffer=my_strtok_buffer;
		strcpy(my_strtok_buffer,buffer);
	        }
	
	sequence_head=my_strtok_buffer;

	if(sequence_head[0]=='\x0')
		return NULL;
	
	token_position=index(my_strtok_buffer,tokens[0]);

	if(token_position==NULL){
		my_strtok_buffer=index(my_strtok_buffer,'\x0');
		return sequence_head;
	        }

	token_position[0]='\x0';
	my_strtok_buffer=token_position+1;

	return sequence_head;

        }


/* fixes compiler problems under Solaris, since strsep() isn't included */
/* this code is taken from the glibc source */
char *my_strsep (char **stringp, const char *delim){
	char *begin, *end;

	begin = *stringp;
	if (begin == NULL)
		return NULL;

	/* A frequent case is when the delimiter string contains only one
	   character.  Here we don't need to call the expensive `strpbrk'
	   function and instead work using `strchr'.  */
	if(delim[0]=='\0' || delim[1]=='\0'){
		char ch = delim[0];

		if(ch=='\0' || begin[0]=='\0')
			end=NULL;
		else{
			if(*begin==ch)
				end=begin;
			else
				end=strchr(begin+1,ch);
			}
		}

	else
		/* Find the end of the token.  */
		end = strpbrk (begin, delim);

	if(end){

		/* Terminate the token and set *STRINGP past NUL character.  */
		*end++='\0';
		*stringp=end;
		}
	else
		/* No more delimiters; this is the last token.  */
		*stringp=NULL;

	return begin;
	}


#ifdef REMOVED_10182007
/* my wrapper for free() */
int my_free(void **ptr){

	if(ptr==NULL)
		return ERROR;

	/* I hate calling free() and then resetting the pointer to NULL, so lets do it together */
	if(*ptr){
		free(*ptr);
		*ptr=NULL;
	        }

	return OK;
        }
#endif


/* open a file read-only via mmap() */
mmapfile *mmap_fopen(char *filename){
	mmapfile *new_mmapfile;
	int fd;
	void *mmap_buf;
	struct stat statbuf;
	int mode=O_RDONLY;

	/* allocate memory */
	if((new_mmapfile=(mmapfile *)malloc(sizeof(mmapfile)))==NULL)
		return NULL;

	/* open the file */
	if((fd=open(filename,mode))==-1)
		return NULL;

	/* get file info */
	if((fstat(fd,&statbuf))==-1){
		close(fd);
		free(new_mmapfile);
		return NULL;
	        }

	/* mmap() the file */
	if((mmap_buf=(void *)mmap(0,statbuf.st_size,PROT_READ,MAP_PRIVATE,fd,0))==MAP_FAILED){
		close(fd);
		free(new_mmapfile);
		return NULL;
	        }

	/* populate struct info for later use */
	new_mmapfile->path=strdup(filename);
	new_mmapfile->fd=fd;
	new_mmapfile->file_size=(unsigned long)(statbuf.st_size);
	new_mmapfile->current_position=0L;
	new_mmapfile->current_line=0L;
	new_mmapfile->mmap_buf=mmap_buf;

	return new_mmapfile;
        }



/* close a file originally opened via mmap() */
int mmap_fclose(mmapfile *temp_mmapfile){

	if(temp_mmapfile==NULL)
		return ERROR;

	/* un-mmap() the file */
	munmap(temp_mmapfile->mmap_buf,temp_mmapfile->file_size);

	/* close the file */
	close(temp_mmapfile->fd);

	/* free memory */
	free(temp_mmapfile);
	
	return OK;
        }



/* gets one line of input from an mmap()'ed file */
char *mmap_fgets(mmapfile *temp_mmapfile){
	char *buf=NULL;
	unsigned long x=0L;
	int len=0;

	if(temp_mmapfile==NULL)
		return NULL;

	/* size of file is 0 bytes */
	if(temp_mmapfile->file_size==0L)
		return NULL;

	/* we've reached the end of the file */
	if(temp_mmapfile->current_position>=temp_mmapfile->file_size)
		return NULL;

	/* find the end of the string (or buffer) */
	for(x=temp_mmapfile->current_position;x<temp_mmapfile->file_size;x++){
		if(*((char *)(temp_mmapfile->mmap_buf)+x)=='\n'){
			x++;
			break;
			}
	        }

	/* calculate length of line we just read */
	len=(int)(x-temp_mmapfile->current_position);

	/* allocate memory for the new line */
	if((buf=(char *)malloc(len+1))==NULL)
		return NULL;

	/* copy string to newly allocated memory and terminate the string */
	memcpy(buf,((char *)(temp_mmapfile->mmap_buf)+temp_mmapfile->current_position),len);
	buf[len]='\x0';

	/* update the current position */
	temp_mmapfile->current_position=x;

	/* increment the current line */
	temp_mmapfile->current_line++;

	return buf;
        }



/* gets one line of input from an mmap()'ed file (may be contained on more than one line in the source file) */
char *mmap_fgets_multiline(mmapfile *temp_mmapfile){
	char *buf=NULL;
	char *tempbuf=NULL;
	char *stripped=NULL;
	int len=0;
	int len2=0;
	int end=0;

	if(temp_mmapfile==NULL)
		return NULL;

	while(1){

		my_free(tempbuf);

		if((tempbuf=mmap_fgets(temp_mmapfile))==NULL)
			break;

		if(buf==NULL){
			len=strlen(tempbuf);
			if((buf=(char *)malloc(len+1))==NULL)
				break;
			memcpy(buf,tempbuf,len);
			buf[len]='\x0';
		        }
		else{
			/* strip leading white space from continuation lines */
			stripped=tempbuf;
			while(*stripped==' ' || *stripped=='\t')
				stripped++;
			len=strlen(stripped);
			len2=strlen(buf);
			if((buf=(char *)realloc(buf,len+len2+1))==NULL)
				break;
			strcat(buf,stripped);
			len+=len2;
			buf[len]='\x0';
		        }

		if(len==0)
			break;

		/* handle Windows/DOS CR/LF */
		if(len>=2 && buf[len-2]=='\r')
			end=len-3;
		/* normal Unix LF */
		else if(len>=1 && buf[len-1]=='\n')
			end=len-2;
		else
			end=len-1;

		/* two backslashes found, so unescape first backslash first and break */
		if(end>=1 && buf[end-1]=='\\' && buf[end]=='\\'){
			buf[end]='\n';
			buf[end+1]='\x0';
			break;
			}

		/* one backslash found, so we should continue reading the next line */
		else if(end>0 && buf[end]=='\\')
			buf[end]='\x0';

		/* else no continuation marker was found, so break */
		else
			break;
	        }

	my_free(tempbuf);

	return buf;
        }



/* get days, hours, minutes, and seconds from a raw time_t format or total seconds */
void get_time_breakdown(unsigned long raw_time,int *days,int *hours,int *minutes,int *seconds){
	unsigned long temp_time;
	int temp_days;
	int temp_hours;
	int temp_minutes;
	int temp_seconds;

	temp_time=raw_time;

	temp_days=temp_time/86400;
	temp_time-=(temp_days * 86400);
	temp_hours=temp_time/3600;
	temp_time-=(temp_hours * 3600);
	temp_minutes=temp_time/60;
	temp_time-=(temp_minutes * 60);
	temp_seconds=(int)temp_time;

	*days=temp_days;
	*hours=temp_hours;
	*minutes=temp_minutes;
	*seconds=temp_seconds;

	return;
	}



/* encodes a string in proper URL format */
char *url_encode(char *input){
	int len,output_len;
	int x,y;
	char temp_expansion[4];
	static int i = 0;
	char* str = encoded_url_string[i];

	/* initialize return string */
	strcpy(str,"");

	if(input==NULL)
		return str;

	len=(int)strlen(input);
	output_len=(int)sizeof(encoded_url_string[0]);

	str[0]='\x0';

	for(x=0,y=0;x<=len && y<output_len-1;x++){

		/* end of string */
		if((char)input[x]==(char)'\x0'){
			str[y]='\x0';
			break;
		        }

		/* alpha-numeric characters and a few other characters don't get encoded */
		else if(((char)input[x]>='0' && (char)input[x]<='9') || ((char)input[x]>='A' && (char)input[x]<='Z') || ((char)input[x]>=(char)'a' && (char)input[x]<=(char)'z') || (char)input[x]==(char)'.' || (char)input[x]==(char)'-' || (char)input[x]==(char)'_'){
			str[y]=input[x];
			y++;
		        }

		/* spaces are pluses */
		else if((char)input[x]<=(char)' '){
			str[y]='+';
			y++;
		        }

		/* anything else gets represented by its hex value */
		else{
			str[y]='\x0';
			if((int)strlen(str)<(output_len-3)){
				sprintf(temp_expansion,"%%%02X",(unsigned int)input[x]);
				strcat(str,temp_expansion);
				y+=3;
			        }
		        }
	        }

	str[sizeof(encoded_url_string[0])-1]='\x0';

	return str;
        }



/* escapes a string used in HTML */
char * html_encode(char *input, int escape_newlines){
	int len,output_len;
	int x,y;
	char temp_expansion[10];

	/* we need up to six times the space to do the conversion */
	len=(int)strlen(input);
	output_len=len*6;
	if((encoded_html_string=(char *)malloc(output_len+1))==NULL)
		return "";

	strcpy(encoded_html_string,"");

	for(x=0,y=0;x<=len;x++){

		/* end of string */
		if((char)input[x]==(char)'\x0'){
			encoded_html_string[y]='\x0';
			break;
		        }

		/* alpha-numeric characters and spaces don't get encoded */
		else if(((char)input[x]==(char)' ') || ((char)input[x]>='0' && (char)input[x]<='9') || ((char)input[x]>='A' && (char)input[x]<='Z') || ((char)input[x]>=(char)'a' && (char)input[x]<=(char)'z'))
			encoded_html_string[y++]=input[x];

		/* newlines turn to <BR> tags */
		else if(escape_newlines==TRUE && (char)input[x]==(char)'\n'){
			strcpy(&encoded_html_string[y],"<BR>");
			y+=4;
			}
		else if(escape_newlines==TRUE && (char)input[x]==(char)'\\' && (char)input[x+1]==(char)'n'){
			strcpy(&encoded_html_string[y],"<BR>");
			y+=4;
			x++;
			}

		/* TODO - strip all but allowed HTML tags out... */

		else if((char)input[x]==(char)'<'){

			if(escape_html_tags==FALSE)
				encoded_html_string[y++]=input[x];
			else{
				encoded_html_string[y]='\x0';
				if((int)strlen(encoded_html_string)<(output_len-4)){
					strcat(encoded_html_string,"&lt;");
					y+=4;
					}
				}
		        }

		else if((char)input[x]==(char)'>'){

			if(escape_html_tags==FALSE)
				encoded_html_string[y++]=input[x];
			else{
				encoded_html_string[y]='\x0';
				if((int)strlen(encoded_html_string)<(output_len-4)){
					strcat(encoded_html_string,"&gt;");
					y+=4;
					}
				}
		        }

		/* for simplicity, all other chars represented by their numeric value */
		else{
			if(escape_html_tags==FALSE)
				encoded_html_string[y++]=input[x];
			else{
				encoded_html_string[y]='\x0';
				sprintf(temp_expansion,"&#%d;",(unsigned char)input[x]);
				if((int)strlen(encoded_html_string)<(output_len-strlen(temp_expansion))){
					strcat(encoded_html_string,temp_expansion);
					y+=strlen(temp_expansion);
					}
				}
		        }
	        }

	encoded_html_string[y++]='\x0';

	return encoded_html_string;
        }



/* strip > and < from string */
void strip_html_brackets(char *buffer){
	register int x;
	register int y;
	register int z;

	if(buffer==NULL || buffer[0]=='\x0')
		return;

	/* remove all occurances in string */
	z=(int)strlen(buffer);
	for(x=0,y=0;x<z;x++){
		if(buffer[x]=='<' || buffer[x]=='>')
			continue;
		buffer[y++]=buffer[x];
	        }
	buffer[y++]='\x0';

	return;
	}



/* escape string for html form usage */
char * escape_string(char *input){
	int len,output_len;
	int x,y;
	char temp_expansion[10];

	/* we need up to six times the space to do the conversion */
	len=(int)strlen(input);
	output_len=len*6;
	if((encoded_html_string=(char *)malloc(output_len+1))==NULL)
		return "";

	strcpy(encoded_html_string,"");

	for(x=0,y=0;x<=len;x++){

		/* end of string */
		if((char)input[x]==(char)'\x0'){
			encoded_html_string[y]='\x0';
			break;
			}

		/* alpha-numeric characters don't get encoded */
		else if(((char)input[x]>='0' && (char)input[x]<='9') || ((char)input[x]>='A' && (char)input[x]<='Z') || ((char)input[x]>=(char)'a' && (char)input[x]<=(char)'z'))
			encoded_html_string[y++]=input[x];

		/* spaces, hyphens, periods, underscores and colons don't get encoded */
		else if(((char)input[x]==(char)' ') || ((char)input[x]==(char)'-') || ((char)input[x]==(char)'.') || ((char)input[x]==(char)'_') || ((char)input[x]==(char)':'))
			encoded_html_string[y++]=input[x];

		/* for simplicity, all other chars represented by their numeric value */
		else{
			encoded_html_string[y]='\x0';
			sprintf(temp_expansion,"&#%d;",(unsigned char)input[x]);
			if((int)strlen(encoded_html_string)<(output_len-strlen(temp_expansion))){
				strcat(encoded_html_string,temp_expansion);
				y+=strlen(temp_expansion);
				}
			}
	        }

	encoded_html_string[y++]='\x0';

	return encoded_html_string;
        }



/* determines the log file we should use (from current time) */
void get_log_archive_to_use(int archive,char *buffer,int buffer_length){
	struct tm *t;

	/* determine the time at which the log was rotated for this archive # */
	determine_log_rotation_times(archive);

	/* if we're not rotating the logs or if we want the current log, use the main one... */
	if(log_rotation_method==LOG_ROTATION_NONE || archive<=0){
		strncpy(buffer,log_file,buffer_length);
		buffer[buffer_length-1]='\x0';
		return;
	        }

	t=localtime(&this_scheduled_log_rotation);

	/* use the time that the log rotation occurred to figure out the name of the log file */
	snprintf(buffer,buffer_length,"%snagios-%02d-%02d-%d-%02d.log",log_archive_path,t->tm_mon+1,t->tm_mday,t->tm_year+1900,t->tm_hour);
	buffer[buffer_length-1]='\x0';

	return;
        }



/* determines log archive to use, given a specific time */
int determine_archive_to_use_from_time(time_t target_time){
	time_t current_time;
	int current_archive=0;

	/* if log rotation is disabled, we don't have archives */
	if(log_rotation_method==LOG_ROTATION_NONE)
		return 0;

	/* make sure target time is rational */
	current_time=time(NULL);
	if(target_time>=current_time)
		return 0;

	/* backtrack through archives to find the one we need for this time */
	/* start with archive of 1, subtract one when we find the right time period to compensate for current (non-rotated) log */
	for(current_archive=1;;current_archive++){

		/* determine time at which the log rotation occurred for this archive number */
		determine_log_rotation_times(current_archive);

		/* if the target time falls within the times encompassed by this archive, we have the right archive! */
		if(target_time>=this_scheduled_log_rotation)
			return current_archive-1;
	        }

	return 0;
        }



/* determines the log rotation times - past, present, future */
void determine_log_rotation_times(int archive){
	struct tm *t;
	int current_month;
	int is_dst_now=FALSE;
	time_t current_time;

	/* negative archive numbers don't make sense */
	/* if archive=0 (current log), this_scheduled_log_rotation time is set to next rotation time */
	if(archive<0)
		return;

	time(&current_time);
	t=localtime(&current_time);
	is_dst_now=(t->tm_isdst>0)?TRUE:FALSE;
	t->tm_min=0;
	t->tm_sec=0;


	switch(log_rotation_method){

	case LOG_ROTATION_HOURLY:
		this_scheduled_log_rotation=mktime(t);
		this_scheduled_log_rotation=(time_t)(this_scheduled_log_rotation-((archive-1)*3600));
		last_scheduled_log_rotation=(time_t)(this_scheduled_log_rotation-3600);
		break;

	case LOG_ROTATION_DAILY:
		t->tm_hour=0;
		this_scheduled_log_rotation=mktime(t);
		this_scheduled_log_rotation=(time_t)(this_scheduled_log_rotation-((archive-1)*86400));
		last_scheduled_log_rotation=(time_t)(this_scheduled_log_rotation-86400);
		break;

	case LOG_ROTATION_WEEKLY:
		t->tm_hour=0;
		this_scheduled_log_rotation=mktime(t);
		this_scheduled_log_rotation=(time_t)(this_scheduled_log_rotation-(86400*t->tm_wday));
		this_scheduled_log_rotation=(time_t)(this_scheduled_log_rotation-((archive-1)*604800));
		last_scheduled_log_rotation=(time_t)(this_scheduled_log_rotation-604800);
		break;

	case LOG_ROTATION_MONTHLY:

		t=localtime(&current_time);
		t->tm_mon++;
		t->tm_mday=1;
		t->tm_hour=0;
		t->tm_min=0;
		t->tm_sec=0;
		for(current_month=0;current_month<=archive;current_month++){
			if(t->tm_mon==0){
				t->tm_mon=11;
				t->tm_year--;
			        }
			else
				t->tm_mon--;
		        }
		last_scheduled_log_rotation=mktime(t);

		t=localtime(&current_time);
		t->tm_mon++;
		t->tm_mday=1;
		t->tm_hour=0;
		t->tm_min=0;
		t->tm_sec=0;
		for(current_month=0;current_month<archive;current_month++){
			if(t->tm_mon==0){
				t->tm_mon=11;
				t->tm_year--;
			        }
			else
				t->tm_mon--;
		        }
		this_scheduled_log_rotation=mktime(t);

		break;
	default:
		break;
	        }

	/* adjust this rotation time for daylight savings time */
	t=localtime(&this_scheduled_log_rotation);
	if(t->tm_isdst>0 && is_dst_now==FALSE)
		this_scheduled_log_rotation=(time_t)(this_scheduled_log_rotation-3600);
	else if(t->tm_isdst==0 && is_dst_now==TRUE)
		this_scheduled_log_rotation=(time_t)(this_scheduled_log_rotation+3600);

	/* adjust last rotation time for daylight savings time */
	t=localtime(&last_scheduled_log_rotation);
	if(t->tm_isdst>0 && is_dst_now==FALSE)
		last_scheduled_log_rotation=(time_t)(last_scheduled_log_rotation-3600);
	else if(t->tm_isdst==0 && is_dst_now==TRUE)
		last_scheduled_log_rotation=(time_t)(last_scheduled_log_rotation+3600);

	return;
        }




/**********************************************************
 *************** COMMON HTML FUNCTIONS ********************
 **********************************************************/

void display_info_table(char *title,int refresh, authdata *current_authdata){
	time_t current_time;
	char date_time[MAX_DATETIME_LENGTH];
	int result;

	/* read program status */
	result=read_all_status_data(get_cgi_config_location(),READ_PROGRAM_STATUS);

	printf("<TABLE CLASS='infoBox' BORDER=1 CELLSPACING=0 CELLPADDING=0>\n");
	printf("<TR><TD CLASS='infoBox'>\n");
	printf("<DIV CLASS='infoBoxTitle'>%s</DIV>\n",title);

	time(&current_time);
	get_time_string(&current_time,date_time,(int)sizeof(date_time),LONG_DATE_TIME);

	printf("最近更新: %s<BR>\n",date_time);
	if(refresh==TRUE)
		printf("每%d秒更新一次<br>\n",refresh_rate);

	printf("Nagios&reg; Core&trade; %s - <A HREF='http://www.nagios.org' TARGET='_new' CLASS='homepageURL'>www.nagios.org</A><BR>\n",PROGRAM_VERSION);

	if(current_authdata!=NULL)
		printf("当前登录账号: <i>%s</i><BR>\n",(!strcmp(current_authdata->username,""))?"?":current_authdata->username);

	if(nagios_process_state!=STATE_OK)
		printf("<DIV CLASS='infoBoxBadProcStatus'>告警: 监控进程未运行!<BR><A HREF='%s?type=%d'>点击</A> 查看详细信息.</DIV>",EXTINFO_CGI,DISPLAY_PROCESS_INFO);

	if(result==ERROR)
		printf("<DIV CLASS='infoBoxBadProcStatus'>告警: 无法读取程序状态信息!</DIV>");

	else{
		if(enable_notifications==FALSE)
			printf("<DIV CLASS='infoBoxBadProcStatus'>- 通知功能被禁用!</DIV>");

		if(execute_service_checks==FALSE)
			printf("<DIV CLASS='infoBoxBadProcStatus'>- 服务检查被禁用!</DIV>");
	        }

	printf("</TD></TR>\n");
	printf("</TABLE>\n");

	return;
        }



void display_nav_table(char *url,int archive){
	char date_time[MAX_DATETIME_LENGTH];
	char archive_file[MAX_INPUT_BUFFER];
	char *archive_basename;

	if(log_rotation_method!=LOG_ROTATION_NONE){
		printf("<table border=0 cellspacing=0 cellpadding=0 CLASS='navBox'>\n");
		printf("<tr>\n");
		printf("<td align=center valign=center CLASS='navBoxItem'>\n");
		if(archive==0){
			printf("最近的归档<br>");
			printf("<a href='%sarchive=1'><img src='%s%s' border=0 alt='最近的归档' title='最近的归档'></a>",url,url_images_path,LEFT_ARROW_ICON);
		        }
		else{
			printf("更早的归档<br>");
			printf("<a href='%sarchive=%d'><img src='%s%s' border=0 alt='更早的归档' title='更早的归档'></a>",url,archive+1,url_images_path,LEFT_ARROW_ICON);
		        }
		printf("</td>\n");

		printf("<td width=15></td>\n");

		printf("<td align=center CLASS='navBoxDate'>\n");
		printf("<DIV CLASS='navBoxTitle'>日志文件导航</DIV>\n");
		get_time_string(&last_scheduled_log_rotation,date_time,(int)sizeof(date_time),LONG_DATE_TIME);
		printf("%s",date_time);
		printf("<br>到<br>");
		if(archive==0)
			printf("现在..");
		else{
			get_time_string(&this_scheduled_log_rotation,date_time,(int)sizeof(date_time),LONG_DATE_TIME);
			printf("%s",date_time);
		        }
		printf("</td>\n");

		printf("<td width=15></td>\n");
		if(archive!=0){

			printf("<td align=center valign=center CLASS='navBoxItem'>\n");
			if(archive==1){
				printf("当前日志<br>");
				printf("<a href='%s'><img src='%s%s' border=0 alt='当前日志' title='当前日志'></a>",url,url_images_path,RIGHT_ARROW_ICON);
			        }
			else{
				printf("更多近期日志<br>");
				printf("<a href='%sarchive=%d'><img src='%s%s' border=0 alt='更多近期日志' title='更多近期日志'></a>",url,archive-1,url_images_path,RIGHT_ARROW_ICON);
			        }
			printf("</td>\n");
		        }
		else
			printf("<td><img src='%s%s' border=0 width=75 height=1></td>\n",url_images_path,EMPTY_ICON);

		printf("</tr>\n");

		printf("</table>\n");
	        }

	/* get archive to use */
	get_log_archive_to_use(archive,archive_file,sizeof(archive_file)-1);

	/* cut the pathname for security, and the remaining slash for clarity */
	archive_basename=(char *)&archive_file;
	if(strrchr((char *)&archive_basename,'/')!=NULL)
		archive_basename=strrchr((char *)&archive_file,'/')+1;

	/* now it's safe to print the filename */
	printf("<BR><DIV CLASS='navBoxFile'>文件: %s</DIV>\n",archive_basename);

	return;
        }



/* prints the additional notes or action url for a hostgroup (with macros substituted) */
void print_extra_hostgroup_url(char *group_name, char *url){
	char input_buffer[MAX_INPUT_BUFFER]="";
	char output_buffer[MAX_INPUT_BUFFER]="";
	char *temp_buffer;
	int in_macro=FALSE;
	hostgroup *temp_hostgroup=NULL;

	if(group_name==NULL || url==NULL)
		return;

	temp_hostgroup=find_hostgroup(group_name);
	if(temp_hostgroup==NULL){
		printf("%s",url);
		return;
	        }

	strncpy(input_buffer,url,sizeof(input_buffer)-1);
	input_buffer[sizeof(input_buffer)-1]='\x0';

	for(temp_buffer=my_strtok(input_buffer,"$");temp_buffer!=NULL;temp_buffer=my_strtok(NULL,"$")){

		if(in_macro==FALSE){
			if(strlen(output_buffer)+strlen(temp_buffer)<sizeof(output_buffer)-1){
				strncat(output_buffer,temp_buffer,sizeof(output_buffer)-strlen(output_buffer)-1);
				output_buffer[sizeof(output_buffer)-1]='\x0';
			        }
			in_macro=TRUE;
			}
		else{

			if(strlen(output_buffer)+strlen(temp_buffer) < sizeof(output_buffer)-1){

				if(!strcmp(temp_buffer,"HOSTGROUPNAME"))
					strncat(output_buffer,url_encode(temp_hostgroup->group_name),sizeof(output_buffer)-strlen(output_buffer)-1);
			        }

			in_macro=FALSE;
		        }
	        }

	printf("%s",output_buffer);

	return;
        }



/* prints the additional notes or action url for a servicegroup (with macros substituted) */
void print_extra_servicegroup_url(char *group_name, char *url){
	char input_buffer[MAX_INPUT_BUFFER]="";
	char output_buffer[MAX_INPUT_BUFFER]="";
	char *temp_buffer;
	int in_macro=FALSE;
	servicegroup *temp_servicegroup=NULL;

	if(group_name==NULL || url==NULL)
		return;

	temp_servicegroup=find_servicegroup(group_name);
	if(temp_servicegroup==NULL){
		printf("%s",url);
		return;
	        }

	strncpy(input_buffer,url,sizeof(input_buffer)-1);
	input_buffer[sizeof(input_buffer)-1]='\x0';

	for(temp_buffer=my_strtok(input_buffer,"$");temp_buffer!=NULL;temp_buffer=my_strtok(NULL,"$")){

		if(in_macro==FALSE){
			if(strlen(output_buffer)+strlen(temp_buffer)<sizeof(output_buffer)-1){
				strncat(output_buffer,temp_buffer,sizeof(output_buffer)-strlen(output_buffer)-1);
				output_buffer[sizeof(output_buffer)-1]='\x0';
			        }
			in_macro=TRUE;
			}
		else{

			if(strlen(output_buffer)+strlen(temp_buffer) < sizeof(output_buffer)-1){

				if(!strcmp(temp_buffer,"SERVICEGROUPNAME"))
					strncat(output_buffer,url_encode(temp_servicegroup->group_name),sizeof(output_buffer)-strlen(output_buffer)-1);
			        }

			in_macro=FALSE;
		        }
	        }

	printf("%s",output_buffer);

	return;
        }



/* include user-defined SSI footers or headers */
void include_ssi_files(char *cgi_name, int type){
	char common_ssi_file[MAX_INPUT_BUFFER];
	char cgi_ssi_file[MAX_INPUT_BUFFER];
	char raw_cgi_name[MAX_INPUT_BUFFER];
	char *stripped_cgi_name;
	int x;

	/* common header or footer */
	snprintf(common_ssi_file,sizeof(common_ssi_file)-1,"%scommon-%s.ssi",physical_ssi_path,(type==SSI_HEADER)?"header":"footer");
	common_ssi_file[sizeof(common_ssi_file)-1]='\x0';

	/* CGI-specific header or footer */
	strncpy(raw_cgi_name,cgi_name,sizeof(raw_cgi_name)-1);
	raw_cgi_name[sizeof(raw_cgi_name)-1]='\x0';
	stripped_cgi_name=strtok(raw_cgi_name,".");
	snprintf(cgi_ssi_file,sizeof(cgi_ssi_file)-1,"%s%s-%s.ssi",physical_ssi_path,(stripped_cgi_name==NULL)?"":stripped_cgi_name,(type==SSI_HEADER)?"header":"footer");
	cgi_ssi_file[sizeof(cgi_ssi_file)-1]='\x0';
	for(x=0;x<strlen(cgi_ssi_file);x++)
		cgi_ssi_file[x]=tolower(cgi_ssi_file[x]);

	if(type==SSI_HEADER){
		printf("\n<!-- Produced by Nagios (http://www.nagios.org).  Copyright (c) 1999-2007 Ethan Galstad. -->\n");
		include_ssi_file(common_ssi_file);
		include_ssi_file(cgi_ssi_file);
	        }
	else{
		include_ssi_file(cgi_ssi_file);
		include_ssi_file(common_ssi_file);
		printf("\n<!-- Produced by Nagios (http://www.nagios.org).  Copyright (c) 1999-2007 Ethan Galstad. -->\n");
	        }

	return;
        }



/* include user-defined SSI footer or header */
void include_ssi_file(char *filename){
	char buffer[MAX_INPUT_BUFFER];
	FILE *fp;
        struct stat stat_result;
        int call_return;

        /* if file is executable, we want to run it rather than print it */
        call_return=stat(filename,&stat_result);

	/* file is executable */
	if(call_return==0 && (stat_result.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))){

		/* must flush output stream first so that output
		   from script shows up in correct place. Other choice
		   is to open program under pipe and copy the data from
		   the program to our output stream.
		*/
		fflush(stdout);

		/* ignore return status from system call. */
		call_return=system(filename);

		return;
	        }

	/* an error occurred trying to stat() the file */
	else if(call_return!=0){

		/* Handle error conditions. Assume that standard posix error codes and errno are available. If not, comment this section out. */
		switch(errno){
		case ENOTDIR: /* - A component of the path is not a directory. */
		case ELOOP: /* Too many symbolic links encountered while traversing the path. */
		case EFAULT: /* Bad address. */
		case ENOMEM: /* Out of memory (i.e. kernel memory). */
		case ENAMETOOLONG: /* File name too long. */
			printf("<br /> %d: %s 的文件状态获取调用返回错误.<br />", errno, filename);
			return;
		case EACCES: /* Permission denied. -- The file should be accessible by nagios. */
			printf("<br /> -%d: %s 的文件权限错误.<br />", errno, filename);
			return;
		case ENOENT: /* A component of the path file_name does not exist, or the path is an empty string. Just return if the file doesn't exist. */
			return;
		default:
			return;
		        }
	        }
	    
	fp=fopen(filename,"r");
	if(fp==NULL)
		return;

	/* print all lines in the SSI file */
	while(fgets(buffer,(int)(sizeof(buffer)-1),fp)!=NULL)
		printf("%s",buffer);

	fclose(fp);
	
	return;
        }


/* displays an error if CGI config file could not be read */
void cgi_config_file_error(char *config_file){

	printf("<H1>出错啦!</H1>\n");

	printf("<P><STRONG><FONT COLOR='RED'>错误: 无法打开CGI配置文件 '%s' !</FONT></STRONG></P>\n",config_file);

	printf("<P>\n");
	printf("要解决上述错误，请检查如下配置:\n");
	printf("</P>\n");

	printf("<P>\n");
	printf("<OL>\n");

	printf("<LI>Nagios的CGI配置文件在合适的路径上。查看错误信息中关于CGI配置文件的位置。示例的CGI配置文件(<b>cgi.cfg</b>)在Nagios源码下的<b>sampl        e-config/</b>目录中。\n");
	printf("<LI>Web服务器有读取Nagios的CGI配置文件的权限。\n");
	printf("</OL>\n");
	printf("</P>\n");

	printf("<P>\n");
	printf("请阅读\"Installing and configuration\"文档后在尝试。如果还有其他错误信息，请发送邮件至邮件列表。可以从<a href='http://www.nagios.org'        >http://www.nagios.org</a>获取更多的信息。\n");
	printf("</P>\n");

	return;
        }



/* displays an error if main config file could not be read */
void main_config_file_error(char *config_file){

	printf("<H1>出错啦!</H1>\n");

	printf("<P><STRONG><FONT COLOR='RED'>错误: 无法打开主配置文件'%s'!</FONT></STRONG></P>\n",config_file);

	printf("<P>\n");
	printf("按如下步骤来解决这个错误:\n");
	printf("</P>\n");

	printf("<P>\n");
	printf("<OL>\n");

	printf("<LI>请你确认主配置文件nagios.cfg被安装在正确的位置。更详细的内容请检查CGI的配置文件中出错信息。一个的样例的主配置文件(<b>nagiso.cfg</        b>)可以在<b>sample-config/</b>目录中找到，它在Nagios的源程序目录下。\n");
	printf("<LI>请确认Web服务程序(Apache)具备正确地权限设置以便于程序读入主配置文件。\n");

	printf("</OL>\n");
	printf("</P>\n");

	printf("<P>\n");
	printf("继续进行前请确认Nagios的配置和文档被正确安装并配置。如果不是这样，请你发一封信到邮件列表中的一个地址中，更多的信息可以查阅<a href='ht        tp://www.nagios.org'>http://www.nagios.org</a>Web站点。\n");
	printf("</P>\n");

	return;
        }


/* displays an error if object data could not be read */
void object_data_error(void){

	printf("<H1>出错啦!</H1>\n");

	printf("<P><STRONG><FONT COLOR='RED'>错误: 无法正确地读出配置文件中的对象数据！</FONT></STRONG></P>\n");

	printf("<P>\n");
	printf("你需要按如下过程来做以解决这个错误：\n");
	printf("</P>\n");

	printf("<P>\n");
	printf("<OL>\n");

	printf("<LI>使用<b>-v</b>命令行参数来校对配置文件配置选项中的错误。(如:nagios -v nagios.config\n");
        printf("<LI>Nagios的日志文件中将列出此次检查出错的内容及状态。\n"); 
        printf("<LI>要首先确认你使用了与主程序相同版本的cgi程序（这影响配置中的数据定义比如例子文件或xdata可阅读格式定义是否相同）。\n"); 

	printf("</OL>\n");
	printf("</P>\n");

	printf("<P>\n");
	printf("继续进行前请确认Nagios的配置和文档被正确安装并配置。如果不是这样，请你发一封信到邮件列表中的一个地址中，更多的信息可以查阅<a href='http://www.nagios.org'>http://www.nagios.org</a>的主页站点。\n");
	printf("</P>\n");

	return;
        }


/* displays an error if status data could not be read */
void status_data_error(void){

	printf("<H1>出错啦!</H1>\n");

	printf("<P><STRONG><FONT COLOR='RED'>错误: 无法获得到主机和服务的状态信息！</FONT></STRONG></P>\n");

	printf("<P>\n");
	printf("<P>通常这个错误信息特别是对于新手而言意味着Nagios并没有正常地运行起来。如果Nagios没被运行了，这是个正常的出错信息。这只是说明CGIs程序无法获得当前正在变动的主机和服务的状态信息。如果你仅仅是安装了Nagios的话，请确认Nagios处于\"starting Nagios\"的状态之中。\n");
	printf("</P>\n");

	printf("<P>\n");
	printf("你需要按如下过程来做以解决这个错误：\n");
	printf("</P>\n");

	printf("<P>\n");
	printf("<OL>\n");

	printf("<LI>检查一下Nagios的日志文件中与Nagios的启动和状态数据相关的日志信息。\n");
        printf("<LI>通常情况下使用命令行选项<b>-v</b>(如, nagios -v nagios.cfg)来验证配置文件的正确性，之后再启动或重启动Nagios。\n");
        printf("<LI>要首先确认你使用了与主程序相同版本的cgi程序（这影响配置中的数据定义比如例子文件或xdata可阅读格式定义是否相同）。\n");


	printf("</OL>\n");
	printf("</P>\n");

	printf("<P>\n");
	printf("确保你在\"Installing and configuration\"中阅读文件之前你的Nagios可以正常运行。如果全都不行，到如下站点上发出你的问题<a href='http://www.nagios.org'>http://www.nagios.org</a>。\n");
	printf("</P>\n");

	return;
        }




/* displays context-sensitive help window */
void display_context_help(char *chid){
	char *icon=CONTEXT_HELP_ICON1;

	if(show_context_help==FALSE)
		return;

	/* change icon if necessary */
	if(!strcmp(chid,CONTEXTHELP_TAC))
		icon=CONTEXT_HELP_ICON2;

	printf("<a href='%s%s.html' target='cshw' onClick='javascript:window.open(\"%s%s.html\",\"cshw\",\"width=550,height=600,toolbar=0,location=0,status=0,resizable=1,scrollbars=1\");return true'><img src='%s%s' border=0 alt='Display context-sensitive help for this screen' title='Display context-sensitive help for this screen'></a>\n",url_context_help_path,chid,url_context_help_path,chid,url_images_path,icon);

	return;
        }



void display_splunk_host_url(host *hst){

	if(enable_splunk_integration==FALSE)
		return;
	if(hst==NULL)
		return;

	printf("<a href='%s?q=search %s' target='_blank'><img src='%s%s' alt='Splunk It' title='Splunk It' border='0'></a>\n",splunk_url,url_encode(hst->name),url_images_path,SPLUNK_SMALL_WHITE_ICON);

	return;
	}



void display_splunk_service_url(service *svc){

	if(enable_splunk_integration==FALSE)
		return;
	if(svc==NULL)
		return;

	printf("<a href='%s?q=search %s%%20",splunk_url,url_encode(svc->host_name));
	printf("%s' target='_blank'><img src='%s%s' alt='Splunk It' title='Splunk It' border='0'></a>\n",url_encode(svc->description),url_images_path,SPLUNK_SMALL_WHITE_ICON);

	return;
	}



void display_splunk_generic_url(char *buf, int icon){
	char *newbuf=NULL;

	if(enable_splunk_integration==FALSE)
		return;
	if(buf==NULL)
		return;

	if((newbuf=(char *)strdup(buf))==NULL)
		return;

	strip_splunk_query_terms(newbuf);

	printf("<a href='%s?q=search %s' target='_blank'>",splunk_url,url_encode(newbuf));
	if(icon>0)
		printf("<img src='%s%s' alt='Splunk It' title='Splunk It' border='0'>",url_images_path,(icon==1)?SPLUNK_SMALL_WHITE_ICON:SPLUNK_SMALL_BLACK_ICON);
	printf("</a>\n");

	free(newbuf);

	return;
	}


/* strip quotes and from string */
void strip_splunk_query_terms(char *buffer){
	register int x;
	register int y;
	register int z;

	if(buffer==NULL || buffer[0]=='\x0')
		return;

	/* remove all occurances in string */
	z=(int)strlen(buffer);
	for(x=0,y=0;x<z;x++){
		if(buffer[x]=='\'' || buffer[x]=='\"' || buffer[x]==';' || buffer[x]==':' || buffer[x]==',' || buffer[x]=='-' || buffer[x]=='=')
			buffer[y++]=' ';
		else
			buffer[y++]=buffer[x];
	        }
	buffer[y++]='\x0';

	return;
	}


/* Opsera patch: This is duplicated from base/logging.c, with some checks removed. 
   Required to place username of user submiting query into nagios.log */
/* write something to the nagios log file */
int write_to_log(char *buffer) {
	FILE *fp;
	time_t log_time;

	fp=fopen(log_file,"a+");
	if(fp==NULL){
		return ERROR;
		}

	time(&log_time);

	/* strip any newlines from the end of the buffer */
	strip(buffer);

	/* write the buffer to the log file */
	fprintf(fp,"[%lu] %s\n",log_time,buffer);

	fclose(fp);

	return OK;
	}

