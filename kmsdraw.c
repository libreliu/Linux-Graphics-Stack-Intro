#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#define MAX_DRM_DEVICES 64

static int find_drm_render_device(void)
{
	drmDevicePtr devices[MAX_DRM_DEVICES] = { NULL };
	int num_devices, fd = -1;

	num_devices = drmGetDevices2(0, devices, MAX_DRM_DEVICES);
	if (num_devices < 0) {
		printf("drmGetDevices2 failed: %s\n", strerror(-num_devices));
		return -1;
	}

	for (int i = 0; i < num_devices && fd < 0; i++) {
		drmDevicePtr device = devices[i];

		if (!(device->available_nodes & (1 << DRM_NODE_RENDER)))
			continue;
		fd = open(device->nodes[DRM_NODE_RENDER], O_RDWR);
	}
	drmFreeDevices(devices, num_devices);

	if (fd < 0)
		printf("no drm device found!\n");

	return fd;
}

int main() {
    int fd = find_drm_render_device();
    if (fd < 0) {
        fprintf(stderr, "Failed to open DRM device: %s\n", strerror(errno));
        return 1;
    }

    drmModeRes *resources = drmModeGetResources(fd);
    if (!resources) {
        fprintf(stderr, "Failed to get DRM resources: %s\n", strerror(errno));
        drmClose(fd);
        return 1;
    }

    printf("Connected CRTCs, encoders, connectors, and outputs:\n");
    for (int i = 0; i < resources->count_connectors; i++) {
        drmModeConnector *connector = drmModeGetConnector(fd, resources->connectors[i]);
        if (connector) {
            printf("Connector %d (type: %d, status: %s):\n", connector->connector_id, connector->connector_type,
                   connector->connection == DRM_MODE_CONNECTED ? "Connected" : "Disconnected");

            for (int j = 0; j < connector->count_encoders; j++) {
                drmModeEncoder *encoder = drmModeGetEncoder(fd, connector->encoders[j]);
                if (encoder) {
                    printf("\tEncoder %d, type: %d, CRTC ID: %d\n", encoder->encoder_id, encoder->encoder_type, encoder->crtc_id);
                    drmModeFreeEncoder(encoder);
                }
            }

            drmModeFreeConnector(connector);
        } else {
            fprintf(stderr, "Failed to get connector %d: %s\n", resources->connectors[i], strerror(errno));
        }
    }

    drmModeFreeResources(resources);
    drmClose(fd);
    return 0;
}