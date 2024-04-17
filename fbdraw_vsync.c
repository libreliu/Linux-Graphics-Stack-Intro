#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

// Function to draw a solid rectangle
void draw_rectangle(uint32_t *fb_ptr, int screen_width, int rect_x, int rect_y,
                    int rect_width, int rect_height, uint32_t color) {
  for (int y = rect_y; y < rect_y + rect_height; y++) {
    for (int x = rect_x; x < rect_x + rect_width; x++) {
      int idx = y * screen_width + x;
      fb_ptr[idx] = color;
    }
  }
}

// vinfo - current screen info; will be updated if modified
int enum_and_set_mode(int fb_fd, struct fb_var_screeninfo *vinfo, int set_mode, int w, int h, int bpp) {
  struct fb_con2fbmap cmap;
  struct fb_var_screeninfo orig_vinfo; // 保存原始屏幕信息以便恢复

  // 保存当前视频模式信息
  memcpy(&orig_vinfo, vinfo, sizeof(struct fb_var_screeninfo));

  // // 获取当前控制台的映射信息
  // if (ioctl(fb_fd, FBIOGET_CON2FBMAP, &cmap)) {
  //   perror("Error reading console framebuffer mapping");
  //   return 0;
  // }

  // 列出所有支持的模式
  printf("Supported modes:\n");
  system("cat /sys/class/graphics/fb0/modes");

  struct fb_var_screeninfo mode;
  if (set_mode) {
    // 设置新的模式（选择一个具体的模式来设置）
    mode.yres = w;
    mode.xres = h;
    mode.bits_per_pixel = bpp;
    if (ioctl(fb_fd, FBIOPUT_VSCREENINFO, &mode)) {
      perror("Error setting variable information");
      ioctl(fb_fd, FBIOPUT_VSCREENINFO, &orig_vinfo); // 恢复原始模式
      close(fb_fd);
      return 0;
    }

    memcpy(vinfo, &mode, sizeof(struct fb_var_screeninfo));
  }

  return 1;
}

int wait_for_vsync(int fb_fd) {
    int arg = 0;  // Argument for FBIO_WAITFORVSYNC

    // FBIO_WAITFORVSYNC takes a pointer to an integer.
    // The integer's value typically does not matter, but it must be valid.
    if (ioctl(fb_fd, FBIO_WAITFORVSYNC, &arg)) {
        perror("Error waiting for vertical sync");
        return 0;
    }

    return 1;
}
int main(int argc, char *argv[]) {
  int set_mode = 0;
  int dest_w, dest_h, dest_bpp;

  if (argc == 4) {
    set_mode = 1;
    dest_w = atoi(argv[1]);
    dest_h = atoi(argv[2]);
    dest_bpp = atoi(argv[3]);
    printf("Modesetting target resolution: %dx%d, %dbpp\n", dest_w, dest_h, dest_bpp);
  } else if (argc != 1) {
    printf("Usage: %s [new_width new_height new_bpp]\n", argv[0]);
  }

  int fb_fd = open("/dev/fb0", O_RDWR);
  if (fb_fd == -1) {
    perror("Error opening framebuffer device");
    return 1;
  }

  struct fb_var_screeninfo vinfo;
  if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo)) {
    perror("Error reading variable information");
    close(fb_fd);
    return 1;
  }

  // Print screen resolution
  printf("Current screen resolution: %dx%d, Bits per Pixel: %d\n", vinfo.xres,
         vinfo.yres, vinfo.bits_per_pixel);

  enum_and_set_mode(fb_fd, &vinfo, set_mode, dest_w, dest_h, dest_bpp);

  int screensize =
      vinfo.yres_virtual * vinfo.xres_virtual * vinfo.bits_per_pixel / 8;
  uint32_t *fbp = (uint32_t *)mmap(0, screensize, PROT_READ | PROT_WRITE,
                                   MAP_SHARED, fb_fd, 0);
  if ((int)fbp == -1) {
    perror("Failed to mmap framebuffer device to memory");
    close(fb_fd);
    return 1;
  }

  // White color
  uint32_t white = 0xFFFFFF;
  uint32_t green = 0x00FF00;

  for (int i = 0; i < 5000000; i++) {
    draw_rectangle(fbp, vinfo.xres_virtual, 100, 100, 200, 150, i % 2 == 0 ? white : green);
    if (!wait_for_vsync(fb_fd)) {
      break;
    }
  }

  // Clear resources
  munmap(fbp, screensize);
  close(fb_fd);
  return 0;
}