/*
  Get lyric from website

*/

#include <curl/curl.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h> /*for tolower and isspace*/
#include "../main.h"

struct MemoryStruct {
    char *memory;
    size_t size;
};

bool ShowLyric(char* singer, char* songName, BarSettings_t* settings) {
    assert(strlen(singer) > 0 && strlen(songName) > 0);
    char outUrl[512] = {0};

    //filter space and word in the black list
    char singerFiltered[64] = {0};
    char songNameFiltered[128] = {0};
    int p = 0;

    while (*singer != '\0') {
        if(strlen(singer) > 4 && strncasecmp(singer,"the ", 4)==0){
            singer += 4;
        }else if (isspace(*singer) || ispunct(*singer)) {
            singer ++;
            continue;
        }else {
            singerFiltered[p] = tolower(*singer);
            p++;
            singer ++; 
        }
    }

    p = 0;
    while (*songName != '\0') {
        if (isspace(*songName)) {
            songName ++;
            continue;
        }else if(*songName == '('){
            while (*songName != ')' && *songName != '\0') {
                songName ++;
            }
            if(*songName == ')') {
                songName ++;
            }
        }else {
            songNameFiltered[p] = tolower(*songName);
            p++;
            songName++;
        }
    }

    strcpy(outUrl, "http://www.azlyrics.com/lyrics/");
    strcat(outUrl, singerFiltered);
    strcat(outUrl, "/");
    strcat(outUrl, songNameFiltered);
    strcat(outUrl, ".html");

//    strcat(outUrl,"\n");  //for show result with UiMsgShow


    downloadFromWeb(outUrl, settings);

    return true;
}


static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
//        BarUiMsg (&app->settings, MSG_Error, "not enough memory (realloc returned NULL)\n");
        printf("not enough memory\n");
        return 0;
    }
 
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

//    printf("Get Data\n\n\n");
    return realsize;
}

int downloadFromWeb(char* url, BarSettings_t* settings)
{
    CURL *curl_handle;
    CURLcode res;
 
 
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
    chunk.size = 0;    /* no data at this point */ 

 
    curl_global_init(CURL_GLOBAL_ALL);
 
    /* init the curl session */ 
    curl_handle = curl_easy_init();
 
    /* specify URL to get */ 
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
 
    /* send all data to this function  */ 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
 
    /* we pass our 'chunk' struct to the callback function */ 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
 
    /* some servers don't like requests that are made without a user-agent
       field, so we provide one */ 
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
 
    /* get it! */ 
    res = curl_easy_perform(curl_handle);
 
    /* check for errors */ 
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }else {
//        BarUiMsg(settings, MSG_INFO, chunk.memory);
        FilteHtml(chunk.memory);
    }
 
    printf ("%s\n", url);
    /* cleanup curl stuff */ 
    curl_easy_cleanup(curl_handle);
 
 
    free(chunk.memory);
    /* we're done with libcurl, so clean it up */ 
    curl_global_cleanup();
 
    return 0;
}

int FilteHtml(char* html) {
    assert(html != NULL);
    assert(strlen(html) > 100);

    char* start = strcasestr(html, "Sorry about that.");
    if (start == NULL) {
        printf ("can not find lyric start position.\n");
        return -1;
    }
    char* end = strcasestr (start, "</div>");
    if (end == NULL) {
        printf ("cannot find lyric end position.\n");
        return -1;
    }

    *end = '\0';

//    fwrite(start, 20, end-start-20,stdout);
    printf("%s", start+22);

    return 0;
}