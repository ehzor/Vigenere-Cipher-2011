/*********************************
 * Authentication code.  Mostly used for just shadowauth().
 *
 * I'm sure this code can be more clean, but...not really going to happen here.
 *********************************/
#ifndef __AUTH_H
#define __AUTH_H

// Files needed to authenticate against /etc/shadow
#include <shadow.h>
#include <crypt.h>

// Has useful functions
#include "global.h"

/**
 * shadowauth()
 * u:   Username to authenticate        [in]
 * p:   Given password for user         [in]
 *
 * Authenticates user against /etc/shadow file.
 *
 * Retuns 1 on success, 0 on failure.
 **/
int shadowauth(const char *u, const char *p){
        // Shadow password structure (see shadow.h)
        struct spwd *spw = NULL;

        // salt = $#$....$ (i.e.: $1$abcdefg)
        char *salt = (char*)malloc(sizeof(char) * 11);
        memset(salt, '\0', 11);

        int ret = 0;

        // Populate shadow structure based on given username
        if((spw = getspnam(u)) != NULL){
                // sp_pwdp = entire password, salt is the first 11 characters
                strncat(salt, (spw->sp_pwdp), 11);

                // If we encrypt p with salt, and its the same as sp_pwdp, success
                if(streq(crypt(p, salt), spw->sp_pwdp))
                        ret = 1;
        }

        free(salt);

        return ret;
}

#endif
