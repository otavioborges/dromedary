#include <stdio.h>
#include "sftpFunctions.h"
using namespace dromedary;

SftpFunctions::SftpFunctions(Sftp *manager){
    m_sessionManager = manager;
}

bool SftpFunctions::Exists(char *path){

}

void SftpFunctions::List(char *path){
    sftp_dir dir;
    sftp_attributes attibutes;

    if(m_sessionManager == NULL)
        return;
        
    dir = sftp_opendir(*m_sessionManager->GetSFTPSession(), path);
    if(!dir)
        return;

    while((attibutes = sftp_readdir(*m_sessionManager->GetSFTPSession(), dir)) != NULL){
        printf("%-20s %10llu %.8o %s(%d)\t%s(%d)\n",
            attibutes->name,
            (long long unsigned int) attibutes->size,
            attibutes->permissions,
            attibutes->owner,
            attibutes->uid,
            attibutes->group,
            attibutes->gid);
        sftp_attributes_free(attibutes);
    }

    sftp_closedir(dir);
}