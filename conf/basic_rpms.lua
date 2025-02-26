-- start led, end led, color
-- set_led_range_to_color(1, 5, YELLOW);

-- led, color
-- set_led_to_color(3, ORANGE);

-- current colors
-- RED
-- GREEN
-- BLUE
-- YELLOW
-- ORANGE

if simdata.rpm >= simdata.maxrpm-500 then
    set_led_range_to_color(1, 6, RED)
elseif simdata.rpm >= 4000 then
    set_led_range_to_color(1, 4, YELLOW)
elseif simdata.rpm >= 3000 then
    set_led_range_to_color(1, 3, GREEN)
elseif simdata.rpm >= 2000 then
    set_led_range_to_rgb_color(1, 2, 0, 255, 0)
    -- set_led_to_rgb_color also available
elseif simdata.rpm >= 1000 then
    set_led_to_color(1, GREEN)
end




