#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

void on_key_event(int code, int value) {
  printf("Key event: code %d, value %d\n", code, value);

  char *key_name = NULL;
  if (code == BTN_LEFT) {
    key_name = "Left button";
  } else if (code == BTN_RIGHT) {
    key_name = "Right button";
  }

  if (key_name != NULL && value == 0) {
    printf("%s released.\n", key_name);
  } else if (key_name != NULL && value == 1) {
    printf("%s pressed.\n", key_name);
  }
}

#define DEV_INPUT_EVENT "/dev/input"

int versionsort(const struct dirent **a, const struct dirent **b) {
  return atoi((*a)->d_name + strlen(DEV_INPUT_EVENT) + 1) -
         atoi((*b)->d_name + strlen(DEV_INPUT_EVENT) + 1);
}

int is_event_device(const struct dirent *dir) {
  return strncmp("event", dir->d_name, 5) == 0;
}

char *scan_devices(char *result) {
  struct dirent **namelist;
  int ndev, i, devnum;
  int max_device = 0;

  ndev = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, versionsort);
  if (ndev <= 0) {
    perror("Cannot scan input devices");
    return NULL;
  }

  printf("Available input devices:\n");
  for (i = 0; i < ndev; i++) {
    char fname[PATH_MAX];
    int fd = -1;
    char name[PATH_MAX] = "???";
    snprintf(fname, sizeof(fname), "%s/%s", DEV_INPUT_EVENT,
             namelist[i]->d_name);
    fd = open(fname, O_RDONLY);
    if (fd < 0) {
      continue;
    }
    if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
      perror("Error getting device name");
    }
    printf("%s: %s\n", fname, name);
    close(fd);
    if (sscanf(namelist[i]->d_name, "event%d", &devnum) != 1) {
      continue;
    }
    if (devnum > max_device) {
      max_device = devnum;
    }
    free(namelist[i]);
  }

  if (max_device == 0) {
    fprintf(stderr, "No input devices found\n");
    return NULL;
  }

  printf("Select the device event number [0-%d]: ", max_device);
  scanf("%d", &devnum);
  if (devnum > max_device || devnum < 0) {
    return NULL;
  }

  snprintf(result, PATH_MAX, "%s/%s%d", DEV_INPUT_EVENT, "event", devnum);
  return result;
}

int main(int argc, char **argv) {
  char device_path[PATH_MAX];
  if (argc == 2) {
    strncpy(device_path, argv[0], sizeof(device_path) - 1);
    device_path[PATH_MAX - 1] = '\0';
  } else if (argc == 1) {
    scan_devices(device_path);
  } else {
    fprintf(stderr, "Usage: %s /dev/input/eventX\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int fd = open(device_path, O_RDONLY);
  if (fd < 0) {
    perror("Cannot open input device");
    exit(EXIT_FAILURE);
  }

  printf("Listening for mouse movements on %s\n", device_path);

  // 监听设备事件
  while (1) {
    struct input_event ev[64];
    int num_events = read(fd, ev, sizeof(ev));
    if (num_events < 0) {
      perror("Error reading from input device");
      break;
    }

    for (int i = 0; i < num_events / sizeof(struct input_event); ++i) {
      if (ev[i].type == EV_KEY) {
        on_key_event(ev[i].code, ev[i].value);
      }
    }
  }

  close(fd);
  return 0;
}
