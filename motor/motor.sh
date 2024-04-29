#!/bin/bash

# Variables
PWM_CHIP=0
PWM_CHANNEL=0
PWM_PIN=18
PERIOD=20000

# Function to start the motor with a specified duty cycle
start_motor() {
    DUTY_CYCLE=$1
    echo $DUTY_CYCLE
    echo $PWM_CHANNEL > /sys/class/pwm/pwmchip$PWM_CHIP/export
    echo $PERIOD > /sys/class/pwm/pwmchip$PWM_CHIP/pwm$PWM_CHANNEL/period
    echo $DUTY_CYCLE > /sys/class/pwm/pwmchip$PWM_CHIP/pwm$PWM_CHANNEL/duty_cycle
    echo 1 > /sys/class/pwm/pwmchip$PWM_CHIP/pwm$PWM_CHANNEL/enable
    echo "Motor started with duty cycle $DUTY_CYCLE"
}

# Function to stop the motor
stop_motor() {
    echo 0 > /sys/class/pwm/pwmchip$PWM_CHIP/pwm$PWM_CHANNEL/enable
    echo $PWM_CHANNEL > /sys/class/pwm/pwmchip$PWM_CHIP/unexport
    echo "Motor stopped"
}

# Function to adjust the duty cycle while the motor is running
adjust_duty_cycle() {
    NEW_DUTY_CYCLE=$1
    echo $NEW_DUTY_CYCLE > /sys/class/pwm/pwmchip$PWM_CHIP/pwm$PWM_CHANNEL/duty_cycle
    echo "Duty cycle adjusted to $NEW_DUTY_CYCLE"
}

# Main logic to handle start, stop, and adjust
case $1 in
    start)
        if [ -z "$2" ]; then
            echo "Please provide a duty cycle to start the motor."
            exit 1
        fi
        start_motor $2
        ;;
    stop)
        stop_motor
        ;;
    adjust)
        if [ -z "$2" ]; then
            echo "Please provide a duty cycle to adjust."
            exit 1
        fi
        adjust_duty_cycle $2
        ;;
    *)
        echo "Usage: $0 {start|stop|adjust} [duty_cycle]"
        exit 1
        ;;
esac
