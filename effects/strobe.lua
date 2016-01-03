local color = hyperion.args.color
local frequency = math.min(100.0, hyperion.args.frequency)

-- Compute the strobe interval
sleepTime = 1000 / frequency

while not hyperion:abort() do
	hyperion:setColor(0,0,0)
	hyperion:sleep(sleepTime)
	hyperion:setColor(color[1], color[2], color[3])
	hyperion:sleep(sleepTime)
end
