#include <rtthread.h>
#include <stm32f10x.h>

#include <rtgui/event.h>
#include <rtgui/rtgui_server.h>

/*
Key_1 PC13
Key_2 PE0
*/
#define key_1_GETVALUE()  	GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)
#define key_2_GETVALUE()   	GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)

static void GPIO_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;

    /* init gpio configuration */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_Init(GPIOE,&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
    GPIO_Init(GPIOC,&GPIO_InitStructure);

	/* beeper definition */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

static struct rt_timer beep_timer;
static void beep_timeout(void* parameter)
{
	GPIO_WriteBit(GPIOB,GPIO_Pin_9, Bit_SET);
}

static void beep(int tick)
{
#if 0
	GPIO_WriteBit(GPIOB,GPIO_Pin_9, Bit_RESET);
	
	rt_timer_control(&beep_timer, RT_TIMER_CTRL_SET_TIME, &tick);
	rt_timer_start(&beep_timer);
#endif
}

static void key_thread_entry(void *parameter)
{
    rt_time_t next_delay;
    struct rtgui_event_kbd kbd_event;

	GPIO_Configuration();

	rt_timer_init(&beep_timer, "beep",
		beep_timeout, RT_NULL, 1, RT_TIMER_FLAG_ONE_SHOT);

    /* init keyboard event */
    RTGUI_EVENT_KBD_INIT(&kbd_event);
    kbd_event.mod  = RTGUI_KMOD_NONE;
    kbd_event.unicode = 0;

    while (1)
    {
        next_delay = 20;
		kbd_event.key = RTGUIK_UNKNOWN;

        kbd_event.type = RTGUI_KEYDOWN;
        if ( key_1_GETVALUE() == 0 )
        {
            rt_thread_delay(next_delay);
            if (key_1_GETVALUE() == 0)
            {
				beep(1);
            	/* HOME key */
                rt_kprintf("key_home\n");
                kbd_event.key  = RTGUIK_HOME;
            }
            else
            {
                rt_kprintf("key_left\n");
                kbd_event.key  = RTGUIK_LEFT;
            }
        }

        if ( key_2_GETVALUE()  == 0 )
        {
            rt_thread_delay(next_delay);
            if (key_2_GETVALUE() == 0)
            {
				beep(1);
            	/* return key */
                rt_kprintf("key_return\n");
                kbd_event.key  = RTGUIK_RETURN;
			}
			else
			{
	            rt_kprintf("key_right\n");
	            kbd_event.key  = RTGUIK_RIGHT;
			}
        }

		if (kbd_event.key != RTGUIK_UNKNOWN)
		{
	        /* post down event */
	        rtgui_server_post_event(&(kbd_event.parent), sizeof(kbd_event));

			beep(2);

			next_delay = 10;
	        /* delay to post up event */
	        rt_thread_delay(next_delay);

	        /* post up event */
	        kbd_event.type = RTGUI_KEYUP;
	        rtgui_server_post_event(&(kbd_event.parent), sizeof(kbd_event));
		}

        /* wait next key press */
        rt_thread_delay(next_delay);
    }
}

void rt_hw_key_init()
{
    rt_thread_t key_tid;
    key_tid = rt_thread_create("key",
                               key_thread_entry, RT_NULL,
                               512, 30, 5);
    if (key_tid != RT_NULL) rt_thread_startup(key_tid);
}
