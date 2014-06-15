{
    "targets": [
        {
            "target_name": "libs/controller",
            "sources": [
                "libs/quaternion.cc",
                "libs/ahrs.cc",
                "libs/pwm/device.cc",
                "libs/pwm/afro_esc.cc",
                "libs/i2c/device.cc",
                "libs/i2c/adxl345.cc",
                "libs/i2c/bmp085.cc",
                "libs/i2c/hmc5883l.cc",
                "libs/i2c/l3g4200d.cc",
                "libs/controller.cc"
            ],
            "cflags": ["-fpermissive"]
        }
    ]
}
