/**
 * @file sensor.c
 * 
 */
#include <stdio.h>

#include "sensor.h"

#include <zephyr.h>
#include <sys/byteorder.h>
#include <sys/crc.h>
#include <drivers/adc.h>
#include <drivers/gpio.h>
#include <hal/nrf_saadc.h>
#include <net/buf.h>

#include <logging/log.h>
LOG_MODULE_DECLARE(hci_uart);

static const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
static struct gpio_callback gpio_cb;
static atomic_t _tilt_count = ATOMIC_INIT(0);

#define TILT_SENSOR_GPIO_PIN 6

static void invoke_gpio_callback_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(gpio_work, invoke_gpio_callback_handler);

static const struct device *adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));

// P0.30/AIN6 Pressure sensor
static const struct adc_channel_cfg channel_0_cfg = {
    .gain = ADC_GAIN_1_4, // 0.6V / (1/4) = 2.4V Input range
    .reference = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40),
    .channel_id = 0,
    .input_positive = NRF_SAADC_INPUT_AIN6,
};
// P0.05/AIN3 Potention meter
static const struct adc_channel_cfg channel_1_cfg = {
    .gain = ADC_GAIN_1_4, // Input range, VDD
    .reference = ADC_REF_VDD_1_4,
    .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40),
    .channel_id = 1,
    .input_positive = NRF_SAADC_INPUT_AIN3,
};

/**
 * Private method
 */

static void invoke_gpio_callback_handler(struct k_work *work)
{
    LOG_INF("TILT SENSOR detected.");
    atomic_inc(&_tilt_count);
}

static void gpio_callback(const struct device *dev, struct gpio_callback *gpio_cb, uint32_t pins)
{
    int ret;
    // Using a delayed work to remove debounse.
    k_work_cancel_delayable(&gpio_work);
    ret = k_work_schedule(&gpio_work, K_MSEC(300));
	if(ret < 0) {
		LOG_ERR("k_work_schedule(), error: %d.", ret);
	}    
}

static int init_adc(void)
{
    int err;

    if(adc_dev == NULL) {
        LOG_ERR("Can not get an adc device.");
        return -1;
    }    

    err = adc_channel_setup(adc_dev, &channel_0_cfg);
    if(err) {
        LOG_ERR("Setting up the ADC channel 0 failed.");
        return err;
    }

    err = adc_channel_setup(adc_dev, &channel_1_cfg);
    if(err) {
        LOG_ERR("Setting up the ADC channel 1 failed.");
        return err;
    }

    return 0;
}

/**
 * Public method
 * 
 */

void init_sensor(void)
{
    int ret;

    LOG_INF("init sensors...");

    // check gpio.
    if(gpio_dev == NULL) {
        LOG_ERR("Can not get an gpio0 device.");
    }    
	if (!device_is_ready(gpio_dev)) {
		LOG_INF("GPIO0 device is not ready.");
	}    

	/* Configure pin as input. */
	ret = gpio_pin_configure(gpio_dev, TILT_SENSOR_GPIO_PIN, GPIO_INPUT | GPIO_PULL_DOWN | GPIO_INT_DEBOUNCE);
	if (ret) {
		LOG_INF("failed gpio_pin_configure(), error: %d.", ret);
	}

    // setting gpio level sense.
	gpio_init_callback(&gpio_cb, gpio_callback, BIT(TILT_SENSOR_GPIO_PIN));
	ret = gpio_add_callback(gpio_dev, &gpio_cb);
	if (ret) {
		LOG_ERR("GPIO_%d add callback error: %d", TILT_SENSOR_GPIO_PIN, ret);
	}
	ret = gpio_pin_interrupt_configure(gpio_dev, TILT_SENSOR_GPIO_PIN, GPIO_INT_EDGE_RISING);
	if (ret) {
		LOG_ERR("GPIO_%d enable callback error: %d", TILT_SENSOR_GPIO_PIN, ret);
	}

    init_adc();
}

int measure(int16_t *p_buffer) 
{
    static struct adc_sequence sequence =  {
        .options      = NULL, 
        .channels     = BIT(0) | BIT(1),
        .buffer       = NULL,
        .buffer_size  = 4,
        .resolution   = 10,
        .oversampling = 0,
        .calibrate    = true
    };

    sequence.buffer = (void *)p_buffer;

    int err;
    err = adc_read(adc_dev, &sequence);
    if(err) {
        LOG_ERR("ADC read failed, error: %d.", err);
        return err;
    }    

    sequence.calibrate = false;

    return 0;
}

int get_tilt_count(void)
{
    return (int)atomic_get(&_tilt_count);
}
