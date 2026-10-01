/****************************************************************
NAME:
    actions.h
DESCRIPTION:
    Action-ID constants dispatched by selectFromMenu() (see
    src/server/ui/actions.cc), the shared handler for keyboard
    shortcuts and the scroll wheel.
****************************************************************/

#pragma once

void selectFromMenu(int id);

#define UNDO                         2312

#define GRAFIC_PERSP                 2002

#define VIEW_FRONT      21
#define VIEW_SIDE       22
#define VIEW_ZOOM_IN    23
#define VIEW_ZOOM_OUT   24
#define VIEW_RESET      25
#define CED_RESET       27

#define LAYER_0         30
#define LAYER_1         31
#define LAYER_2         32
#define LAYER_3         33
#define LAYER_4         34
#define LAYER_5         35
#define LAYER_6         36
#define LAYER_7         37
#define LAYER_8         38
#define LAYER_9         39
#define LAYER_10        40
#define LAYER_11        41
#define LAYER_12        42
#define LAYER_13        43
#define LAYER_14        44
#define LAYER_15        45
#define LAYER_16        46
#define LAYER_17        47
#define LAYER_18        48
#define LAYER_19        49
#define LAYER_20        50
#define LAYER_21        51
#define LAYER_22        52
#define LAYER_23        53
#define LAYER_24        54
#define LAYER_ALL       60

#define DETECTOR1               4001
#define DETECTOR2               4002
#define DETECTOR3               4003
#define DETECTOR4               4004
#define DETECTOR5               4005
#define DETECTOR6               4006
#define DETECTOR7               4007
#define DETECTOR8               4008
#define DETECTOR9               4009
#define DETECTOR10              4010
#define DETECTOR11              4011
#define DETECTOR12              4012
#define DETECTOR13              4013
#define DETECTOR14              4014
#define DETECTOR15              4015
#define DETECTOR16              4016
#define DETECTOR17              4017
#define DETECTOR18              4018
#define DETECTOR19              4019
#define DETECTOR20              4020
#define DETECTOR21              4021
#define DETECTOR22              4022
#define DETECTOR23              4023
#define DETECTOR24              4024
#define DETECTOR25              4025
#define DETECTOR26              4026
#define DETECTOR27              4027
#define DETECTOR28              4028
#define DETECTOR29              4029
#define DETECTOR30              4030
#define DETECTOR31              4031
#define DETECTOR32              4032
#define DETECTOR33              4033
#define DETECTOR34              4034
#define DETECTOR35              4035
#define DETECTOR36              4036
#define DETECTOR37              4037
#define DETECTOR38              4038
#define DETECTOR39              4039
#define DETECTOR40              4040

#define DETECTOR_ALL            4100

#define TOGGLE_PHI_PROJECTION   5000
#define TOGGLE_Z_PROJECTION     5001
