/*
 * Webpages.h
 *
 *  Created on: 28 de mai de 2018
 *      Author: makara
 */

#ifndef MAIN_CONFIG_WEBPAGES_H_
#define MAIN_CONFIG_WEBPAGES_H_


#include "../mongoose/WebService.h"

void init_cgi(WebService *ws);
void stations_cgi(HTTPData *d);
void config_cgi(HTTPData *d);
void save_cgi(HTTPData *d);
void reboot_cgi(HTTPData *d);
void screen_cgi(HTTPData *d);


#endif /* MAIN_CONFIG_WEBPAGES_H_ */
