#!/bin/bash

PWM_CHIP=0
PWM_CHANNEL=0
PWM_PIN=18

PERIOD=20000
DUTY_CYCLE=$1

echo $DUTY_CYCLE

echo $PWM_CHANNEL > /sys/class/pwm/pwmchip$PWM_CHIP/export

echo $PERIOD > /sys/class/pwm/pwmchip$PWM_CHIP/pwm$PWM_CHANNEL/period

echo $DUTY_CYCLE > /sys/class/pwm/pwmchip$PWM_CHIP/pwm$PWM_CHANNEL/duty_cycle

echo 1 > /sys/class/pwm/pwmchip$PWM_CHIP/pwm$PWM_CHANNEL/enable

echo "check motor"

sleep 10

echo 0 > /sys/class/pwm/pwmchip$PWM_CHIP/pwm$PWM_CHANNEL/enable

echo "off"

echo $PWM_CHANNEL > /sys/class/pwm/pwmchip$PWM_CHIP/unexport
