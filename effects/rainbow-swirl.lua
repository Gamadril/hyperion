-- Get the parameters
local rotationTime = hyperion.args['rotation-time'] or 3000
local brightness = hyperion.args['brightness'] or 255
local saturation = hyperion.args['saturation'] or 255
local reverse = hyperion.args['reverse'] or false

-- Check parameters
rotationTime = math.max(100, rotationTime)
brightness = math.max(0, math.min(brightness, 255))
saturation = math.max(0, math.min(saturation, 255))

-- Initialize the led data
local ledData = {}
for i = 1, hyperion.ledCount do
	local hue = (i-1) * 359 / hyperion.ledCount
	ledData[i] = colors.hsv2rgb(hue, saturation, brightness)
end

-- Calculate the sleep time and rotation increment
local increment = 3
local sleepTime = rotationTime / hyperion.ledCount
while sleepTime < 50 do
	increment = increment * 2
	sleepTime = sleepTime * 2
end
increment = increment % hyperion.ledCount

-- Start the write data loop
while not hyperion:abort() do
	hyperion:setColor(ledData)
	for i = 1, increment do
		local removed
		-- Switch direction if needed
		if reverse == true then
			removed = table.remove(ledData, 1)
			table.insert(ledData, removed)
		else
			removed = table.remove(ledData)
			table.insert(ledData, 1, removed)
		end
	end
	hyperion:sleep(sleepTime)
end

