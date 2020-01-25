//#include <device/serial.h>
enum klog_level {KLOG1};

void klog(enum klog_level level, char *format, ...)
{
	(void)level; (void) format;
	switch(level)
	{

		
		default:
			;	
	}	

}
