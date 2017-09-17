#include "util.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>

// change to toggle debug statements on and off
#define debug debug_on

// based on https://github.com/dvdhrm/docs

// opens the given card (such as /dev/dri/card0), checks to make sure all is good and returns the file descriptor
int modeset_open(string cardDevPath)
{
	// open the card path
	int fd = open(cardDevPath.c_str(), O_RDWR | O_CLOEXEC);
	ASSERT(fd >= 0, return -1);
	
	// make sure the path supports dumb (hardware independent) buffers
	uint64_t has_dumb;
	ASSERT_ELSE(drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) >= 0 && has_dumb, return -1);
	
	return fd
}

struct ModesetDevice
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t stride = 0;
	uint32_t size = 0;
	uint32_t handle = 0;
	uint8_t *map = nullptr;

	drmModeModeInfo mode = {0};
	uint32_t fb = 0;
	uint32_t conn = 0;
	uint32_t crtc = 0;
	drmModeCrtc * saved_crtc = nullptr;
};

vector<unique_ptr<ModesetDevice>> modeset_list;

int modeset_find_crtc(int fd, drmModeRes * resource, drmModeConnector * connector, ModesetDevice * device)
{
	drmModeEncoder * encoder = nullptr;
	int32_t crtc = -1;

	// first try the currently conected encoder+crtc
	if (connector->encoder_id)
		encoder = drmModeGetEncoder(fd, conn->encoder_id);

	if (encoder)
	{
		if (encoder->crtc_id) {
			crtc = encoder->crtc_id;
			for (auto i: modeset_list) {
				if (i->crtc == crtc) {
					crtc = -1;
					break;
				}
			}

			if (crtc >= 0) {
				drmModeFreeEncoder(encoder);
				device->crtc = crtc;
				return 0;
			}
		}

		drmModeFreeEncoder(encoder);
	}

	/* If the connector is not currently bound to an encoder or if the
	 * encoder+crtc is already used by another connector (actually unlikely
	 * but lets be safe), iterate all other available encoders to find a
	 * matching CRTC. */
	for (int i = 0; i < connector->count_encoders; ++i) {
		encoder = drmModeGetEncoder(fd, conn->encoders[i]);
		ASSERT_ELSE(encoder, continue);
		// cannot retrieve encoder

		// iterate all global CRTCs
		for (int j = 0; j < resource->count_crtcs; ++j) {
			// check whether this CRTC works with the encoder
			if (!(enc->possible_crtcs & (1 << j)))
				continue;

			// check that no other device already uses this CRTC
			crtc = res->crtcs[j];
			for (iter = modeset_list; iter; iter = iter->next) {
				if (iter->crtc == crtc) {
					crtc = -1;
					break;
				}
			}

			/* we have found a CRTC, so save it and return */
			if (crtc >= 0) {
				drmModeFreeEncoder(enc);
				dev->crtc = crtc;
				return 0;
			}
		}

		drmModeFreeEncoder(enc);
	}

	fprintf(stderr, "cannot find suitable CRTC for connector %u\n",
		conn->connector_id);
	return -ENOENT;
}

int modeset_setup_dev(int fd, drmModeRes * resource, drmModeConnector * connector, ModesetDevice * device)
{
	// check if a monitor is connected
	if (connector->connection != DRM_MODE_CONNECTED) {
		debug("ignoring unused connector " + to_string(connector->connector_id));
		return -ENOENT;
	}
	
	// check if there is at least one valid mode
	ASSERT_ELSE(connector->count_modes > 0, return -EFAULT);
	
	// copy the mode information into our device structure
	memcpy(&device->mode, &connector->modes[0], sizeof(device->mode));
	device->width = connector->modes[0].hdisplay;
	device->height = connector->modes[0].vdisplay;
	debug("mode for connector " + to_string(conn->connector_id) + " is " + to_string(dev->width) + "x" + to_string(dev->height));
	
	// find a CRTC for this connector
	int ret = modeset_find_crtc(fd, resource, connector, device);
	//make sure there is a valid CRTC for connector
	ASSERT_ELSE(ret == 0, return ret);
	
	// create a framebuffer for this CRTC
	ret = modeset_create_fb(fd, device);
	// make sure framebuffer created successfully
	ASSERT_ELSE(ret == 0, return ret);
	
	return 0;
}

void modeset_prepare(int fd)
{
	drmModeRes * resource;

	// retrieve resources
	resource = drmModeGetResources(fd);
	ASSERT_ELSE(resource, return);

	// iterate all connectors
	for (int i = 0; i < resource->count_connectors; ++i)
	{
		// get information for each connector
		drmModeConnector * connector = drmModeGetConnector(fd, res->connectors[i]);
		ASSERT_ELSE(connector, continue);

		// create a device structure
		auto device = make_unique<ModesetDevice>();
		device->conn = connector->connector_id;

		// call helper function to prepare this connector
		int ret = modeset_setup_dev(fd, res, conn, dev);
		if (ret) {
			if (ret != -ENOENT) {
				errno = -ret;
				fprintf(stderr, "cannot setup device for connector %u:%u (%d): %m\n",
					i, res->connectors[i], errno);
			}
			drmModeFreeConnector(connector);
			continue;
		}

		// free connector data and link device into global list
		drmModeFreeConnector(connector);
		modeset_list.push_back(device);
	}

	// free resources again
	drmModeFreeResources(resource);
}

int main()
{
	debug("DRM simple starting");
	
	debug("all done");
}
