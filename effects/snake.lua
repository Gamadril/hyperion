-- Get the parameters
local rotationTime = math.max(100, hyperion.args['rotation-time'])
local color = hyperion.args.color
local percentage = math.max(1, math.min(hyperion.args.percentage, 100))

-- Process parameters
local factor = percentage / 100.0
local hsv = colors.rgb2hsv(color[1], color[2], color[3])

-- Initialize the led data
local snakeLeds = math.max(1, math.floor(hyperion.ledCount * factor))
ledData = {}

for i = 1, (hyperion.ledCount-snakeLeds) do
	ledData[i] = {0, 0, 0};
end

for i = 1, snakeLeds do
	local rgb = colors.hsv2rgb(hsv[1], hsv[2], hsv[3]/i)
	ledData[#ledData+1] = rgb
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
	local first = ledData[1]
	for i = 1, (hyperion.ledCount-1) do
		ledData[i] = ledData[i+1]
	end
	ledData[hyperion.ledCount] = first
	hyperion:sleep(sleepTime)
end
