import qbs

CppApplication {
    consoleApplication: true
    files: "src/main.cpp"
    Group{
        name:"bcm2835"
        files:[
            "src/bcm2835/bcm2835.c",
            "src/bcm2835/bcm2835.h",
        ]
    }
    Group{
        name:"wiringPi"
        files:[
            "src/wiringPi/ads1115.c",
            "src/wiringPi/ads1115.h",
            "src/wiringPi/drcSerial.c",
            "src/wiringPi/drcSerial.h",
            "src/wiringPi/max31855.c",
            "src/wiringPi/max31855.h",
            "src/wiringPi/max5322.c",
            "src/wiringPi/max5322.h",
            "src/wiringPi/mcp23008.c",
            "src/wiringPi/mcp23008.h",
            "src/wiringPi/mcp23016.c",
            "src/wiringPi/mcp23016.h",
            "src/wiringPi/mcp23016reg.h",
            "src/wiringPi/mcp23017.c",
            "src/wiringPi/mcp23017.h",
            "src/wiringPi/mcp23s08.c",
            "src/wiringPi/mcp23s08.h",
            "src/wiringPi/mcp23s17.c",
            "src/wiringPi/mcp23s17.h",
            "src/wiringPi/mcp23x08.h",
            "src/wiringPi/mcp23x0817.h",
            "src/wiringPi/mcp3002.c",
            "src/wiringPi/mcp3002.h",
            "src/wiringPi/mcp3004.c",
            "src/wiringPi/mcp3004.h",
            "src/wiringPi/mcp3422.c",
            "src/wiringPi/mcp3422.h",
            "src/wiringPi/mcp4802.c",
            "src/wiringPi/mcp4802.h",
            "src/wiringPi/pcf8574.c",
            "src/wiringPi/pcf8574.h",
            "src/wiringPi/pcf8591.c",
            "src/wiringPi/pcf8591.h",
            "src/wiringPi/piHiPri.c",
            "src/wiringPi/piThread.c",
            "src/wiringPi/sn3218.c",
            "src/wiringPi/sn3218.h",
            "src/wiringPi/softPwm.c",
            "src/wiringPi/softPwm.h",
            "src/wiringPi/softServo.c",
            "src/wiringPi/softServo.h",
            "src/wiringPi/softTone.c",
            "src/wiringPi/softTone.h",
            "src/wiringPi/sr595.c",
            "src/wiringPi/sr595.h",
            "src/wiringPi/wiringPi.c",
            "src/wiringPi/wiringPi.h",
            "src/wiringPi/wiringPiI2C.c",
            "src/wiringPi/wiringPiI2C.h",
            "src/wiringPi/wiringPiSPI.c",
            "src/wiringPi/wiringPiSPI.h",
            "src/wiringPi/wiringSerial.c",
            "src/wiringPi/wiringSerial.h",
            "src/wiringPi/wiringShift.c",
            "src/wiringPi/wiringShift.h",
            "src/wiringPi/wpiExtensions.c",
            "src/wiringPi/wpiExtensions.h",
        ]
    }

    cpp.enableExceptions:true
    cpp.cxxLanguageVersion:"c++11"
    cpp.defines:["BCM2835_NO_DELAY_COMPATIBILITY"]
    cpp.includePaths:[".", "src", "src/wiringPi"]
    cpp.commonCompilerFlags:["-pthread"]
    cpp.dynamicLibraries:["rt", "pthread"]
    //cpp.linkerFlags:["-flto"]
    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}
