#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h> 
#include <fcntl.h>
#include <sys/stat.h>

bool module_daemon(void);
