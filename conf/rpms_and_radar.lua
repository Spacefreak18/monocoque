-- configuration
-- set these accordingly
right_leds_start = 7
right_num_leds = 42
left_leds_start = 49
left_num_leds = 42
--

max_radius = 10 -- radius is hardcoded in simapi
proxcars = simdata.proxcars -- also hard coded in simapi

rightsideset = false
leftsideset = false

-- right side
litleds = 0
i = 1
while(i <= proxcars and rightsideset == false) do
    if simdata.pd[i].theta > 40 and simdata.pd[i].theta < 130 then
        dist = math.abs(90 - simdata.pd[i].theta)
        perct = (90 - dist)/90
        litleds = math.ceil(perct * right_num_leds)

        color_perct = simdata.pd[i].radius/10
        yellow = math.floor(color_perct * 255)
        local rgb = (255 << 16) | (yellow << 8) | (0 << 0)
        if simdata.pd[1].theta >= 90 then
            -- car is behind
            set_led_range_to_rgb_color(right_leds_start, right_leds_start + litleds, rgb)
        else
            set_led_range_to_rgb_color(right_leds_start + (right_num_leds - litleds), right_leds_start + right_num_leds, rgb)
        end
        rightsideset = true
    end
    i = i + 1
end

-- left side
litleds = 0
i = 1
while(i <= proxcars and leftsideset == false) do
    if simdata.pd[i].theta > 230 and simdata.pd[i].theta < 320 then
        dist = math.abs(270 - simdata.pd[1].theta)
        perct = (90 - dist)/90
        litleds = math.ceil(perct * left_num_leds)

        color_perct = simdata.pd[i].radius/10
        yellow = math.floor(color_perct * 255)
        local rgb = (255 << 16) | (yellow << 8) | (0 << 0)
        if simdata.pd[1].theta <= 270 then
            -- car is behind
            set_led_range_to_rgb_color(left_leds_start, left_leds_start + litleds, rgb)
        else
            set_led_range_to_rgb_color(left_leds_start + (left_num_leds - litleds), left_leds_start + left_num_leds, rgb)
        end
        leftsideset = true
    end
    i = i + 1
end


-- rpm stuff
if simdata.rpm > 0 and simdata.maxrpm > 0 then
    rpmmargin = .05*simdata.maxrpm;
    rpminterval = (simdata.maxrpm-rpmmargin) / 6;

    litleds = 0
    for i = 1,6 do
        if simdata.rpm >= (rpminterval * i) then
            litleds = i;
        end
    end

    color = GREEN
    if litleds > 3 and litleds < 6 then
        color = YELLOW
    end
    if litleds >= 6 then
        color = RED
    end
    if litleds >=  6 then

        if simdata.mtick % 2 == 0 then
            set_led_range_to_color(1, litleds, color)
        else
            led_clear_all()
        end
    else
        set_led_range_to_color(1, litleds, color)
    end
end
