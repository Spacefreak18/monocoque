-- from simapi.h
 --   SIMAPI_FLAG_GREEN            = 0,
 --   SIMAPI_FLAG_YELLOW           = 1,
 --   SIMAPI_FLAG_RED              = 2,
 --   SIMAPI_FLAG_CHEQUERED        = 3,
 --   SIMAPI_FLAG_BLUE             = 4,
 --   SIMAPI_FLAG_WHITE            = 5,
 --   SIMAPI_FLAG_BLACK            = 6,
 --   SIMAPI_FLAG_BLACK_WHITE      = 7,
 --   SIMAPI_FLAG_BLACK_ORANGE     = 8,
 --   SIMAPI_FLAG_ORANGE           = 9

led_clear_all()

if simdata.rpm > 0 and simdata.maxrpm > 0 then
    rpmmargin = .05*simdata.maxrpm;
    rpminterval = (simdata.maxrpm-rpmmargin) / 14;

    litleds = 0
    for i = 1,14 do
        if simdata.rpm >= (rpminterval * i) then
            litleds = i;
        end
    end

    color = GREEN
    if litleds > 5 and litleds < 12 then
        color = YELLOW
    end
    if litleds >= 12 then
        color = RED
    end
    set_led_range_to_color(59, 59+litleds, color)
end

if simdata.playerflag == 0 then
     set_led_range_to_color(4, 7, GREEN)
end


if simdata.playerflag == 1 then
     set_led_range_to_color(4, 7, YELLOW)
end


if simdata.playerflag == 4 then
     set_led_range_to_color(4, 7, BLUE)
end
