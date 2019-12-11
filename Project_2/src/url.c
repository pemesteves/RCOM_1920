#include "url.h"

#include <string.h>

int parseURL(char* url, URL *parsed_url){
    if(strstr(url, "ftp://") == NULL){
        printf("INPUT ERROR: The url needs to start with 'ftp://' !\n\n");
        return -1;
    }

    char* at_position;

    //Path received by argument
    content path; 

    //FIND @ TO CHECK IF THE url HAS THE USER AND PASSWORD
    if((at_position = strchr(url, '@')) != NULL){
        at_position++;
        if(strchr(at_position, '@') != NULL){
            printf("INPUT ERROR: The url can't have more than one '@' !\n\n");
            return -1;
        }

        char* two_points_position;
        if((two_points_position = strchr(url+6, ':')) != NULL){
            if(two_points_position >= at_position){
                printf("INPUT ERROR: The url should have the character ':' separating the username from the password !\n\n");
                return -1;
            }

            char* slash_position;
            if((slash_position = strchr(at_position, '/')) != NULL){
                if(sscanf(url, "ftp://%[^:]:%[^@]@%[^/]/%s", parsed_url->user, parsed_url->password, parsed_url->host, path) != 4){
                    printf("INPUT ERROR: Expected 4 arguments in the url !\n\n");
                    return -1;
                }

            } else{
                printf("INPUT ERROR: The url should have a '/' separating the host from the url-path !\n\n");
                return -1;
            }

        } else{
            printf("INPUT ERROR: The url should have the character ':' separating the username from the password !\n\n");
            return -1;
        }
    }else{ //IF THERE ISN'T A @ WE ASSUME THAT THE USER DON'T NEED TO LOG IN
        char* slash_position;
        if((slash_position = strchr(url, '/')) != NULL){
            if(sscanf(url, "ftp://%[^/]/%s", parsed_url->host, path) != 2){
                printf("INPUT ERROR: Expected 2 arguments in the url !\n\n");
                return -1;
            }

            strcpy(parsed_url->user, "");
            strcpy(parsed_url->password, "");

        } else{
            printf("INPUT ERROR: The url should have a '/' separating the host from the url-path !\n\n");
            return -1;
        }
    }

    //Last slash is the used to parse the directory and the filename from the received path
    int last_slash = 0;

    for(int i = 0; i < strlen(path); i++){ //Checking where the last slash is
        if(path[i] == '/'){
            last_slash = i + 1;
        }
    }

    //Copying the path 
    if(strncpy(parsed_url->path, path, last_slash) == NULL){
        printf("Error parsing the path ! \n\n");
        return -1;
    }

    //Copying the filename 
    if(strncpy(parsed_url->filename, path + last_slash, strlen(path) - last_slash + 1) == NULL){
        printf("Error parsing the path ! \n\n");
        return -1;
    }

    return 0;
}