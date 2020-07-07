#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct
{
	char *face;
	long read_bytes;
	long send_bytes;
} NetInfo;

char *read_net_info()
{
	FILE *net_fd;
	char *file_path = "/proc/net/dev";
	char *buff = calloc(2048, sizeof(char));
	int res_size = 2048;
	char *res = calloc(2048, sizeof(char));
	net_fd = fopen(file_path, "r");
	if (net_fd == NULL)
	{
		printf("Open failed!\n");
		exit(-1);
	}
	int n = 0;
	while (!feof(net_fd))
	{
		if (fgets(buff, 2047, net_fd) != NULL)
		{
			n++;
			if (n > 2)
			{
				if (strlen(res) + strlen(buff) > res_size)
				{
					res = realloc(res, res_size + 2048);
					res_size += 2048;
				}
				strcat(res, buff);
			}
		}
	}
	clearerr(net_fd);
	if (net_fd != NULL)
	{
		fclose(net_fd);
	}
	free(buff);
	return res;
}

void list_netface(char *_net_info)
{
	char *net_info = _net_info;
	char face[256];

	while (net_info != NULL && *net_info != '\0')
	{
		int i = 0;
		while (*net_info != ':' && i < 256)
		{
			if (*net_info != ' ')
			{
				face[i] = *net_info;
				i++;
			}
			net_info++;
		}
		face[i] = '\0';
		printf("%s\n", face);
		net_info = strstr(net_info, "\n");
		net_info++;
	}
}

NetInfo *net_info(char *face)
{
	NetInfo *net_info = calloc(1, sizeof(NetInfo));
	net_info->face = face;
	char *net_info_str = read_net_info();
	char *tmp = net_info_str;
	tmp = strstr(tmp, face);
	if (tmp == NULL)
	{
		printf("No netface %s found!\n", face);
		exit(-3);
	}
	tmp += strlen(face) + 2;
	char *number_str;
	int n = 0;
	number_str = strtok(tmp, " ");
	while (n < 9 && number_str)
	{
		if (n == 0)
		{
			long bytes_receive = atol(number_str);
			net_info->read_bytes = bytes_receive;
		}
		if (n == 8)
		{
			long bytes_send = atol(number_str);
			net_info->send_bytes = bytes_send;
		}
		number_str = strtok(NULL, " ");
		n++;
	}

	free(net_info_str);
	return net_info;
}

void speed_info(char *face, int interval, int human)
{
	char *units[] = {
		"B/S",
		"KiB/S",
		"MiB/S",
		"GiB/S",
		"TiB/S",
		"PiB/S"
	};
	NetInfo *info1 = net_info(face);
	sleep(interval);
	NetInfo *info2 = net_info(face);
	long read_speed = (info2->read_bytes - info1->read_bytes) / interval;
	long send_speed = (info2->send_bytes - info1->send_bytes) / interval;
	float read_speed_h = read_speed;
	float send_speed_h = send_speed;
	int ru= 0;
	while (read_speed_h >= 1024 && ru < 6)
	{
		read_speed_h = read_speed_h / 1024;
		ru++;
	}
	int su = 0;
	while (send_speed_h >= 1024 && su < 6)
	{
		send_speed_h = send_speed_h / 1024;
		su++;
	}

	printf("%s: up %.2f %s, down %.2f %s\n", face, send_speed_h, units[su], read_speed_h, units[ru]);
	free(info1);
	free(info2);
}

void print_usage()
{
	char usage[] =
		"Options:\n \
	    	--list     		 -l list all interfaces\n \
		--device 		 -d specify the interface\n \
		--count 		 -c tshow n counts\n \
		--interval		 -i interval between two sample\n \
		--human-readable -h print speed like 1K 234M 2G etc\n";
	printf("%s\n", usage);
	exit(0);
}

int main(int argc, char *argv[])
{
	int list = 0;
	int count = 1;
	int interval = 1;
	int human = 0;
	char *face = calloc(256, sizeof(char));

	if (argc <= 1)
	{
		print_usage();
	}

	for (int i = 1; i < argc; i++)
	{
		char *arg = argv[i];
		if (strcmp(arg, "--list") == 0 || strcmp(arg, "-l") == 0)
		{
			list = 1;
		}
		else if (strcmp(arg, "--count") == 0 || strcmp(arg, "-c") == 0)
		{
			count = atoi(argv[++i]);
			if (count == -1)
			{
				count = __INT32_MAX__;
			}
		}
		else if (strcmp(arg, "--interval") == 0 || strcmp(arg, "-i") == 0)
		{
			interval = atoi(argv[++i]);
			if (interval <= 0)
			{
				interval = 1;
			}
		}
		else if (strcmp(arg, "--human-readable") == 0 || strcmp(arg, "-h") == 0)
		{
			human = 1;
		}
		else if (strcmp(arg, "--device") == 0 || strcmp(arg, "-d") == 0)
		{
			strcpy(face, argv[++i]);
		}
		else
		{
			printf("Unknown option %s\n", arg);
			print_usage();
		}
	}

	if (list)
	{
		char *net_info = read_net_info();
		list_netface(net_info);
		free(net_info);
		exit(0);
	} 
	else
	{
		if (!face || strcmp(face, "") == 0 || face == NULL)
		{
			printf("No device specified!\n");
			exit(-1);
		}
		for (int i = 0; i < count; i++)
		{
			speed_info(face, interval, human);
		}
		free(face);
	}
	
}
