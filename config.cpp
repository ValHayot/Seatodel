extern "C"{
#include <iniparser.h>
}
#include <linux/limits.h>
#include <sys/stat.h>
#include "config.h"
#include "logger.h"
#include "passthrough.h"

struct config sea_config;

char * get_sea_home()
{
    char* sea_home;
    sea_home = getenv("SEA_HOME");
    if(sea_home == NULL)
    {
      printf("SEA_HOME is not defined\n");
      exit(1);
    }
    return sea_home;
}

char * get_config_file()
{
    char * sea_home = get_sea_home();
    static char config_file[PATH_MAX];
    strcpy(config_file, sea_home);
    strcat(config_file,"/sea.ini");
    return config_file;
}

void create_config_file(char * config_file)
{
    char * sea_home = get_sea_home();
    FILE * ini ;
    if ((ini=fopen(config_file, "w"))==NULL) {
        printf("ERROR: cannot create %s\n", config_file);
        exit(1);
    }

    fprintf(ini,
            "#\n"
            "# Sea configuration\n"
            "#\n"
            "[Sea]\n"
            "mount_dir = %s/mount_dir ; # use absolute paths\n"
            "log_file = %s/sea.log ;\n"
            "log_level = 1 ; # DEBUG=4, INFO=3, WARNING=2, ERROR=1, NONE=0 (use <= 3 tests)\n"
            "n_sources = 1 ;\n"
            "source_0 = %s/source ; # use absolute paths\n"
            "\n", sea_home, sea_home, sea_home);
    fclose(ini);
}

void parse_config()
{
    char * config_file = get_config_file();
    bool exists = (((funcptr_access) libc_access)(config_file, F_OK ) != -1);
    if(!exists)
    {
        create_config_file(config_file);
        printf("WARNING: Couldn't find config file %s so I created it"
                         " for you (you should really check it).\n", config_file);
    }
    dictionary * config_dict = iniparser_load(config_file);
    if(config_dict == 0)
    {
        printf("Cannot load config file %s\n", config_file);
        exit(1);
    }
    if((sea_config.n_sources = iniparser_getint(config_dict, "sea:n_sources", 0))==0)
    {
         printf("Missing n_sources in config file %s\n", config_file);
         exit(1);
    }
    sea_config.source_mounts = new char* [sea_config.n_sources];
    for(int i = 0 ; i <  sea_config.n_sources ; i++)
    {
        char source_name[15];
        sprintf(source_name, "sea:source_%d", i);
        //sea_config.source_mounts[i] = new char[PATH_MAX];
        if((sea_config.source_mounts[i] = (char *) iniparser_getstring(config_dict, source_name, NULL))==0)
        {
            printf("Missing %s in config file %s\n", source_name, config_file);
            exit(1);
        }
        // add '/' to the end of source mount path if missing
        //if (sea_config.source_mounts[i][strlen(sea_config.source_mounts[i])-1] != '/')
        //    strcat(sea_config.source_mounts[i], "/");
    }
    if((sea_config.mount_dir = (char *) iniparser_getstring(config_dict, "sea:mount_dir", NULL))==0)
    {
        printf("Missing mount_dir in config file %s\n", config_file);
        exit(1);
    }
    if((sea_config.log_file = (char *) iniparser_getstring(config_dict, "sea:log_file", NULL))==0)
    {
        printf("Missing log_file in config file %s\n", config_file);
        exit(1);
    }
    if((sea_config.max_fs = (unsigned long) iniparser_getlongint(config_dict, "sea:max_fs", 0))==0)
    {
        printf("Missing max_fs in config file %s\n", config_file);
        exit(1);
    }

    // add '/' to the end of mount path
    //if (sea_config.mount_dir[strlen(sea_config.mount_dir)-1] != '/')
    //    strcat(sea_config.mount_dir, "/");
    sea_config.log_level = iniparser_getint(config_dict, "sea:log_level", 1);
    sea_config.n_threads = iniparser_getint(config_dict, "sea:n_threads", 1);
    sea_config.parsed = true;
}

config get_sea_config()
{
    if(!sea_config.parsed)
    {
        parse_config();
    }

    return sea_config;
}
