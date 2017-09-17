#include "util.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>

// change to toggle debug statements on and off
#define debug debug_on

// much of this is from http://betteros.org/tut/graphics1.php#dumb

int main()
{
	debug("DRM simple starting");
	
	debug("opening card file descriptor");
	int dri_fd  = open("/dev/dri/card0",O_RDWR); // assumes you want to use card0
	debug("dri_fd: " + to_string(dri_fd));
	
	debug("becoming master of the DRI device");
	ioctl(dri_fd, DRM_IOCTL_SET_MASTER, 0);
	
	debug("getting connections");
	struct drm_mode_card_res resource = {0};
	//Get resource counts
	ioctl(dri_fd, DRM_IOCTL_MODE_GETRESOURCES, &resource);
	int connectorNum = resource.count_connectors;
	// the parenthesis make it guaranteed to be zeroed
	uint64_t * res_fb_buf = new uint64_t[connectorNum]();
	uint64_t * res_crtc_buf = new uint64_t[connectorNum]();
	uint64_t * res_conn_buf = new uint64_t[connectorNum]();
	uint64_t * res_enc_buf = new uint64_t[connectorNum]();
	resource.fb_id_ptr=(uint64_t)res_fb_buf;
	resource.crtc_id_ptr=(uint64_t)res_crtc_buf;
	resource.connector_id_ptr=(uint64_t)res_conn_buf;
	resource.encoder_id_ptr=(uint64_t)res_enc_buf;
	//Get resource IDs
	ioctl(dri_fd, DRM_IOCTL_MODE_GETRESOURCES, &resource);
	
	for (int i=0; i<connectorNum; i++)
	{
		struct drm_mode_modeinfo conn_mode_buf[20]={0};
		uint64_t	conn_prop_buf[20]={0},
					conn_propval_buf[20]={0},
					conn_enc_buf[20]={0};

		struct drm_mode_get_connector conn={0};

		conn.connector_id=res_conn_buf[i];

		ioctl(dri_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);	//get connector resource counts
		conn.modes_ptr=(uint64_t)conn_mode_buf;
		conn.props_ptr=(uint64_t)conn_prop_buf;
		conn.prop_values_ptr=(uint64_t)conn_propval_buf;
		conn.encoders_ptr=(uint64_t)conn_enc_buf;
		ioctl(dri_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);	//get connector resource IDs
	}

	
	delete[] res_fb_buf;
	delete[] res_crtc_buf;
	delete[] res_conn_buf;
	delete[] res_enc_buf;
	
	debug("all done");
}
