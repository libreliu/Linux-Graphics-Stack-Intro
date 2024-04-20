#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#pragma region print
void print_fb_fix_screeninfo(struct fb_fix_screeninfo *info) {
  if (info == NULL) {
    printf("The fb_fix_screeninfo pointer is NULL.\n");
    return;
  }

  printf("id: %s\n", info->id);
  printf("smem_start: %lu\n", info->smem_start);
  printf("smem_len: %u\n", info->smem_len);
  printf("type: %u\n", info->type);
  printf("type_aux: %u\n", info->type_aux);
  printf("visual: %u\n", info->visual);
  printf("xpanstep: %u\n", info->xpanstep);
  printf("ypanstep: %u\n", info->ypanstep);
  printf("ywrapstep: %u\n", info->ywrapstep);
  printf("line_length: %u\n", info->line_length);
  printf("mmio_start: %lu\n", info->mmio_start);
  printf("mmio_len: %u\n", info->mmio_len);
  printf("accel: %u\n", info->accel);
  printf("capabilities: %u\n", info->capabilities);
  printf("reserved: [%u, %u]\n", info->reserved[0], info->reserved[1]);
}

void print_fb_bitfield(struct fb_bitfield *bf) {
  if (bf == NULL) {
    printf("Bitfield is NULL.\n");
    return;
  }
  printf("offset: %u, length: %u, msb_right: %u\n", bf->offset, bf->length,
         bf->msb_right);
}

void print_fb_var_screeninfo(struct fb_var_screeninfo *info) {
  if (info == NULL) {
    printf("The fb_var_screeninfo pointer is NULL.\n");
    return;
  }

  printf("xres: %u, yres: %u\n", info->xres, info->yres);
  printf("xres_virtual: %u, yres_virtual: %u\n", info->xres_virtual,
         info->yres_virtual);
  printf("xoffset: %u, yoffset: %u\n", info->xoffset, info->yoffset);
  printf("bits_per_pixel: %u\n", info->bits_per_pixel);
  printf("grayscale: %u\n", info->grayscale);
  printf("Red bitfield: ");
  print_fb_bitfield(&info->red);
  printf("Green bitfield: ");
  print_fb_bitfield(&info->green);
  printf("Blue bitfield: ");
  print_fb_bitfield(&info->blue);
  printf("Transp bitfield: ");
  print_fb_bitfield(&info->transp);
  printf("nonstd: %u\n", info->nonstd);
  printf("activate: %u\n", info->activate);
  printf("height: %u, width: %u\n", info->height, info->width);
  printf("accel_flags: %u\n", info->accel_flags);
  printf("pixclock: %u\n", info->pixclock);
  printf("left_margin: %u, right_margin: %u\n", info->left_margin,
         info->right_margin);
  printf("upper_margin: %u, lower_margin: %u\n", info->upper_margin,
         info->lower_margin);
  printf("hsync_len: %u, vsync_len: %u\n", info->hsync_len, info->vsync_len);
  printf("sync: %u\n", info->sync);
  printf("vmode: %u\n", info->vmode);
  printf("rotate: %u\n", info->rotate);
  printf("colorspace: %u\n", info->colorspace);
  printf("reserved: [%u, %u, %u, %u]\n", info->reserved[0], info->reserved[1],
         info->reserved[2], info->reserved[3]);
}

#pragma endregion print

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
int enum_and_set_mode(int fb_fd, struct fb_var_screeninfo *vinfo, int set_mode,
                      int w, int h, int bpp) {
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

int main(int argc, char *argv[]) {
  int set_mode = 0;
  int dest_w, dest_h, dest_bpp;

  if (argc == 4) {
    set_mode = 1;
    dest_w = atoi(argv[1]);
    dest_h = atoi(argv[2]);
    dest_bpp = atoi(argv[3]);
    printf("Modesetting target resolution: %dx%d, %dbpp\n", dest_w, dest_h,
           dest_bpp);
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

  struct fb_fix_screeninfo finfo;
  if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo)) {
    perror("Error reading fixed information");
    close(fb_fd);
    return 1;
  }


  // Print screen resolution
  printf("Current screen resolution: %dx%d, Bits per Pixel: %d\n", vinfo.xres,
         vinfo.yres, vinfo.bits_per_pixel);

  enum_and_set_mode(fb_fd, &vinfo, set_mode, dest_w, dest_h, dest_bpp);

  printf("=> fb_var_screeninfo:\n");
  print_fb_var_screeninfo(&vinfo);

  printf("=> fb_fix_screeninfo:\n");
  print_fb_fix_screeninfo(&finfo);

  int screensize =
      vinfo.yres_virtual * vinfo.xres_virtual * vinfo.bits_per_pixel / 8;
  uint32_t *fbp = (uint32_t *)mmap(0, screensize, PROT_READ | PROT_WRITE,
                                   MAP_SHARED, fb_fd, 0);
  if ((void *)fbp == MAP_FAILED) {
    perror("Failed to mmap framebuffer device to memory");
    close(fb_fd);
    return 1;
  }

  // White color
  uint32_t white = 0xFFFFFF;
  uint32_t green = 0x00FF00;

  for (int i = 0; i < 5; i++) {
    // Let's draw a rectangle from (100, 100) with width=200 and height=150
    draw_rectangle(fbp, vinfo.xres_virtual, 100, 100, 200, 150,
                   i % 2 == 0 ? white : green);
    usleep(500000); // sleep for 0.5 sec
  }

  // Clear resources
  munmap(fbp, screensize);
  close(fb_fd);
  return 0;
}