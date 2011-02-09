osgLua.loadWrapper("osgText")

local knownFonts = {
	["DroidSans"] = "assets/fonts/droid-fonts/DroidSans.ttf",
	["DroidSansBold"] = "assets/fonts/droid-fonts/DroidSans-Bold.ttf",
	["DroidSansMono"] = "assets/fonts/droid-fonts/DroidSansMono.ttf",
	["DroidSerifBold"] = "assets/fonts/droid-fonts/DroidSerif-Bold.ttf",
	["DroidSerifBoldItalic"] = "assets/fonts/droid-fonts/DroidSerif-BoldItalic.ttf",
	["DroidSerifItalic"] = "assets/fonts/droid-fonts/DroidSerif-Italic.ttf",
	["DroidSerif"] = "assets/fonts/droid-fonts/DroidSerif-Regular.ttf",
	["DroidSerifRegular"] = "assets/fonts/droid-fonts/DroidSerif-Regular.ttf"
}

local fontCache = {}
function Font(filename)
	if knownFonts[filename] then
		filename = knownFonts[filename]
	end
	if fontCache[filename] then
		return fontCache[filename]
	end
	local f = osgLua.loadObjectFile(filename)
	local c = osgLua.getTypeInfo(f)
	if not c.name == "osgText::Font" then
		error(string.format("Could not load a font from %s - loaded %s instead!", filename, c.name), 2)
	end
	fontCache[filename] = f
	return f
end

function verifyFont(obj)
	local c = osgLua.getTypeInfo(obj)
	if not c then
		error(string.format("Not even an osgLua type: %s", type(f)), 2)
	end
	if not c.name == "osgText::Font" then
		error(string.format("Not a font: %s instead!", c.name), 2)
	end
end

local defaultFont
function setDefaultFont(f)
	if type(f) == "string" then
		f = Font(f)
	end
	verifyFont(f)
	defaultFont = f
end
function TextGeode(arg)
	if type(arg) ~= "table" then
		arg = {arg}
	end
	if arg.font then
		verifyFont(arg.font)
	end
	local color = arg.color or osg.Vec4(0.9,0.8,0.7,1.0)
    local lineHeight = arg.lineHeight or 0.45
    local pos = arg.position or {2.5, -1.5, -7.0}
    local topLineY = pos[2]
    local X = pos[1]
    local Z = pos[3]
    local geode = osg.Geode()
    for i, v in ipairs(arg) do
        local thisLine = osgText.Text()
        thisLine:setFont(arg.font or defaultFont)
        thisLine:setCharacterSize(lineHeight)
        thisLine:setPosition(osg.Vec3(X,topLineY - i * lineHeight,Z))
        thisLine:setText(v)
        thisLine:setColor(color)
        geode:addDrawable(thisLine)
    end
	return geode
end