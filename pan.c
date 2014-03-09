#include "stdio.h"
#include <sys/ioctl.h>
#include <linux/videodev2.h>

int pantilt(int dev, int pan, int tilt, int reset)
{
	struct v4l2_ext_control xctrls[2];
    struct v4l2_ext_controls ctrls;

    if (reset > 0) {
		switch (reset)
		{
			case 1:
				xctrls[0].id = V4L2_CID_PAN_RESET;
				xctrls[0].value = 1;
				break;

			case 2:
				xctrls[0].id = V4L2_CID_TILT_RESET;
				xctrls[0].value = 1;
				break;

		}
		ctrls.count = 1;
		ctrls.controls = xctrls;
	} else {
                xctrls[0].id = V4L2_CID_PAN_RELATIVE;
                xctrls[0].value = pan;
                xctrls[1].id = V4L2_CID_TILT_RELATIVE;
                xctrls[1].value = tilt;
                ctrls.count = 2;
                ctrls.controls = xctrls;
        }
	if ( ioctl(dev, VIDIOC_S_EXT_CTRLS, &ctrls) < 0 )
	{
        perror("VIDIOC_S_EXT_CTRLS - Pan/Tilt error. Are the extended controls available?\n");
		return -1;
	} else {
            printf("PAN/TILT Success");
        }
	return 0;
}
int main() {
	printf("Hello, pan-tilting test\n");
        int dev;
        dev = open("/dev/video1", 0);

        if (dev < 0) {
            printf ("Can't open device file");
        }

        pantilt(dev,0,0,2);
        close(dev);
	printf("Goodbye\n");
}
