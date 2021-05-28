# ESP Watering Timer

## Introduction

Thank you for visiting this repository. ESP Watering Timer can water to plant even when you are not at that place (requires pre-installed solenoid value). you are at the first step of Smart farm.

## Wiring

Wiring is pretty much easy. just connect pushbutton to GPIO14(D5), relay to GPIO12(D6). and connect relay to your solenoid valve

## Features

1. through Webserver, you can easily access nodeMCU module.

   1. you can manually turn on/ off water. (schedule will be supported later version)

   2. webpage indicates two information:

      1. when starts watering, webpage show how much time left until stop.(unfortunately, the limitation of this platform, you have to refresh the page manually.)
      2. it shows how long have been since you last watered.(this is not stable because the data for this feature is calculated with internal function millis().)

   3. you can water up to 30min at once in order to prevent solenoid valve from overheat.

   4. setting watering time will be supported later version.

      