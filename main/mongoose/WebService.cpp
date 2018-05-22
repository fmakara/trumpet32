/*
 * WebService.cpp
 *
 *  Created on: 21 de mai de 2018
 *      Author: makara
 */

#include "WebService.h"

#include <stdio.h>
#include <string.h>

WebService::WebService(const char *port) {
	char servicename[32];
	mg_mgr_init(&mgr, NULL);

	list = NULL;
	nc = mg_bind(&mgr, port, mg_ev_handler);
	if (nc == NULL) {
		printf("Error setting up listener for port %s!\n",port);
		return;
	}
	nc->user_data = (void*)this;
	mg_set_protocol_http_websocket(nc);

	sprintf(servicename,"WebService_%s",port);
	xTaskCreatePinnedToCore(task, servicename, 16000, this, 1, &handle, 0);
}

void WebService::task(void *arg){
	WebService *ws = (WebService*)arg;
	/* Processing events */
	while (1) {
		mg_mgr_poll(&ws->mgr, 1000);
	}
}


void WebService::addCgi(const char *page, void (*cb)(HTTPData*), const char *type){
	struct Page **last, *next;
	for(last = &list ; *last!=NULL ; last = &((*last)->next));
	next = (struct Page*)malloc(sizeof(struct Page));
	next->type = TYPE_CGI;
	strncpy(next->page, page, 31);
	if(type==NULL)next->internalPath[0]=0;
	else strncpy(next->internalPath, type, 31);
	next->page[31]=0;
	next->internalPath[31]=0;
	next->cb = cb;
	next->next = NULL;
	*last = next;
	printf("%p %p\n",last, &list);
	printf("%s loaded\n",next->page);
}

void WebService::addSpiffs(const char *page, const char *path){
	struct Page **last, *next;
	for(last = &list ; *last!=NULL ; last = &((*last)->next));
	next = (struct Page*)malloc(sizeof(struct Page));
	next->type = TYPE_SPIFFS;
	strncpy(next->page, page, 31);
	strncpy(next->internalPath, path, 31);
	next->page[31]=0;
	next->internalPath[31]=0;
	next->cb = NULL;
	next->next = NULL;
	*last = next;
	printf("%s loaded\n",page);
}

int WebService::w_strcmp(const char *wild, const char *s2, int size){
	int pw=0, ps=0;
	if(size<0)size=strlen(s2);
	while(ps<size){
		if(wild[pw]=='*'){
			pw++;
			ps++;
			while(ps<size && wild[pw]!=s2[ps])ps++;
		}else if(wild[pw]==0){
			break;
		}else if(wild[pw]!=s2[ps]){
			return 1;
		}else{
			pw++;
			ps++;
		}
	}
	return !(wild[pw]==0 && ps>=size);
}

void WebService::w_strfix(char *to, const char *wildFrom, const char *wildTo, const char *from, int size){
	int pt=0, pf=0, pwt=0, pwf=0;
	if(size<0)size=strlen(from);
	while(wildTo[pwt]!=0){
		if(wildTo[pwt]=='*'){
			while(wildFrom[pwf]!='*' && wildFrom[pwf]!=0){
				pwf++;
				pf++;
			}
			if(wildFrom[pwf]==0)break;
			pwf++;
			pwt++;
			while(from[pf]!=wildFrom[pwf] && pf<size){
				to[pt] = from[pf];
				pf++;
				pt++;
			}
		}else{
			to[pt]=wildTo[pwt];
			pwt++;
			pt++;
		}
	}
	to[pt] = 0;
}

void WebService::mg_ev_handler(struct mg_connection *nc, int ev, void *p) {
	WebService *ws = (WebService*)nc->user_data;
	if(ev==MG_EV_HTTP_REQUEST){
		struct http_message *hm = (struct http_message *) p;
		struct Page *p;
		bool pending = true;
		for(p=ws->list ; p!=NULL && pending ; p=p->next){
			if(!w_strcmp(p->page,hm->uri.p,(int)hm->uri.len)){
				switch(p->type){
				case TYPE_CGI:
					printf("CGI %s",p->page);
					if(p->internalPath[0]==0 || !w_strcmp(p->internalPath,hm->method.p,(int)hm->method.len)){
						p->cb(new HTTPData(hm,nc));
						pending = false;
					}
					break;
				case TYPE_SPIFFS:
					if( !w_strcmp("GET",hm->method.p,(int)hm->method.len)){
						char replacement[64];
						w_strfix(replacement,p->page,p->internalPath,hm->uri.p,(int)hm->uri.len);
				    	struct stat sta;
						if (stat(replacement, &sta) != 0) {
					    	mg_printf(nc, "HTTP/1.0 404 OK\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: 22\r\n\r\nArquivo nao encontrado");
					    	nc->flags |= MG_F_SEND_AND_CLOSE;
					    }else{
							FILE *f = fopen(replacement, "rb");
					    	mg_printf(nc, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %lu\r\n\r\n",sta.st_size);
					    	ws->sendingData.insert(std::pair<sock_t,FILE*>(nc->sock,f));
					    }
						pending = false;
					}
					break;
				}
			}
		}
	}else if(ev==MG_EV_SEND){
		char mem[1000];
		int read;
		FILE *f = ws->sendingData[nc->sock];
		if(f!=NULL){
			read = fread(mem,1,1000,f);
			mg_send(nc,mem,read);
			if(read<1000){
				fclose(f);
				ws->sendingData.erase(nc->sock);
				nc->flags |= MG_F_SEND_AND_CLOSE;
			}
		}
	}else if(ev==MG_EV_CLOSE){
		FILE *f = ws->sendingData[nc->sock];
		if(f!=NULL){
			fclose(f);
		}
	}
}
