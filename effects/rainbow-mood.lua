-- Get the parameters
local rotationTime = hyperion.args['rotation-time'] or 30000
local brightness = hyperion.args.brightness or 255
local saturation = hyperion.args.saturation or 255
local reverse = hyperion.args.reverse or false

-- Check parameters
rotationTime = math.max(100, rotationTime)
brightness = math.max(0, math.min(brightness, 255))
saturation = math.max(0, math.min(saturation, 255))

-- Calculate the sleep time and hue increment
local sleepTime = 100
local hueIncrement = sleepTime * 1000 / rotationTime

-- Switch direction if needed
if reverse == true then
	hueIncrement = -hueIncrement
end

-- Start the write data loop
local hue = 0
while not hyperion:abort() do
	local rgb = colors.hsv2rgb(hue, saturation, brightness)
	hyperion:setColor(rgb[1], rgb[2], rgb[3])
	hue = (hue + hueIncrement) % 359
	hyperion:sleep(sleepTime)
end

