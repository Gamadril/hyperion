local speed = hyperion.args.speed or 1.0
local fadeFactor = hyperion.args.fadeFactor or 0.7
local color = hyperion.args.color or {255,0,0}

speed = math.max(0.0001, speed)
fadeFactor = math.max(0.0, math.min(fadeFactor, 1.0))

-- Initialize the led data
local width = 25
local imageData = {} --bytearray(width * (0,0,0))
imageData[1] = color
for i=2, width do
    imageData[i] = {0,0,0}
end


-- Calculate the sleep time and rotation increment
local increment = 1
local sleepTime = 1000 / (speed * width)
while sleepTime < 50 do
	increment = increment * 2
	sleepTime = sleepTime * 2
end

--Start the write data loop
local position = 0
local direction = 1
while not hyperion:abort() do
	hyperion:setImage(width, 1, imageData)

	-- Move data into next state
	for i=1, increment do
		position = position + direction
		if position == 0 then
			position = 1
			direction = 1
		elseif position == width+1 then
			position = width
			direction = -1
		end

		-- Fade the old data
		for j=1, width do
			imageData[j] = {
                math.floor(fadeFactor * imageData[j][1]),
                math.floor(fadeFactor * imageData[j][2]),
                math.floor(fadeFactor * imageData[j][3])
            }
		end

		-- Insert new data
		imageData[position] = color
	end

	-- Sleep for a while
	hyperion:sleep(sleepTime)
end
