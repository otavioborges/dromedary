#define SFTP_LOG_ID     "SFTP"

#include <string.h>
#include <stdio.h>
#include "sftp.h"
#include "LogInfo.h"
using namespace dromedary;
using namespace std;

Sftp::Sftp(char *server, int port){
    strcpy(m_server, server);
    m_port = port;
    m_key = NULL;
    m_isConnected = false;
}

bool Sftp::LoadKey(char *path){
    
}

bool Sftp::Connect(char * username){
    char message[200];
    int result = 0;

    if(m_isConnected)
        return true;
    
    if(m_key == NULL){
        LogInfo::AddEntry(SFTP_LOG_ID, "No keys were loaded for authentication", LogLevel::ERROR);
        return false;
    }

    if(strlen(username) <= 0){
        LogInfo::AddEntry(SFTP_LOG_ID, "Invalid username provided", LogLevel::ERROR);
        return false;
    }

    LogInfo::AddEntry(SFTP_LOG_ID, "Initiating SSH session", LogLevel::DEBUG);
    m_sshSession = ssh_new();
    if(m_sshSession == NULL){
        strcpy(message, "Error initiating SSH session: ");
        strcat(message, ssh_get_error(m_sshSession));
        LogInfo::AddEntry(SFTP_LOG_ID, message, LogLevel::ERROR);
        return false;
    }

    ssh_options_set(m_sshSession, SSH_OPTIONS_HOST, m_server);
    // ssh_options_set(m_sshSession, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(m_sshSession, SSH_OPTIONS_PORT, &m_port);

    LogInfo::AddEntry(SFTP_LOG_ID, "Trying to connect to SSH server", LogLevel::DEBUG);
    result = ssh_connect(m_sshSession);
    if(result != SSH_OK){
        strcpy(message, "Error connecting to SSH session: ");
        strcat(message, ssh_get_error(m_sshSession));
        LogInfo::AddEntry(SFTP_LOG_ID, message, LogLevel::ERROR);

        ssh_free(m_sshSession);
        return false;
    }

    LogInfo::AddEntry(SFTP_LOG_ID, "Authenticating using kiven key", LogLevel::DEBUG);
    result = ssh_userauth_publickey(m_sshSession, username, m_key);
    if(result != SSH_OK){
        strcpy(message, "Error on authentication: ");
        strcat(message, ssh_get_error(m_sshSession));
        LogInfo::AddEntry(SFTP_LOG_ID, message, LogLevel::ERROR);
        
        ssh_free(m_sshSession);
        return false;
    }

    LogInfo::AddEntry(SFTP_LOG_ID, "Initiating SFTP session", LogLevel::DEBUG);
    m_sftpSession = sftp_new(m_sshSession);
    if(m_sftpSession == NULL){
        strcpy(message, "Error initiating SFTP session: ");
        strcat(message, ssh_get_error(m_sshSession));
        LogInfo::AddEntry(SFTP_LOG_ID, message, LogLevel::ERROR);
        
        ssh_free(m_sshSession);
        return false;
    }

    LogInfo::AddEntry(SFTP_LOG_ID, "Connecting to SFTP server", LogLevel::DEBUG);
    result = sftp_init(m_sftpSession);
    if(result != SSH_OK){
        sprintf(message, "Error connecting to SFTP. Code: %i", sftp_get_error(m_sftpSession));
        LogInfo::AddEntry(SFTP_LOG_ID, message, LogLevel::ERROR);
        
        sftp_free(m_sftpSession);
        ssh_free(m_sshSession);
        return false;
    }

    // If we reach it here all systems GO
    LogInfo::AddEntry(SFTP_LOG_ID, "Successfully connected to SFTP server", LogLevel::DEBUG);
    m_isConnected = true;
    return true;
}

void Sftp::Disconnect(void){
    if(m_isConnected){
        sftp_free(m_sftpSession);

        ssh_disconnect(m_sshSession);
        ssh_free(m_sshSession);

    LogInfo::AddEntry(SFTP_LOG_ID, "Disconnected from server", LogLevel::DEBUG);
        m_isConnected = false;
    }
}

bool Sftp::isConnected(void){
    return m_isConnected;
}