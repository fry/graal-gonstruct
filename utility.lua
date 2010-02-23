-- Replace format strings in str with their corresponding entry in dict,
-- the way python formats strings.
-- TODO: don't ignore format code
function python_format(str, dict)
  for k, v in pairs(dict) do
    str = string.gsub(str, "%%%(" .. k .. "%).", v)
  end
  return str
end

-- Captures output of a command
function os.capture(cmd, raw)
  local f = assert(io.popen(cmd, 'r'))
  local s = assert(f:read('*a'))
  f:close()
  if raw then return s end
  s = string.gsub(s, '^%s+', '')
  s = string.gsub(s, '%s+$', '')
  s = string.gsub(s, '[\n\r]+', ' ')
  return s
end

-- Uses pkg-config to specify build and link options for the passed packages
-- list
function pkg_config(packages)
  packages = table.concat(packages, ' ')

  configuration "vs*"
    buildoptions { os.capture("pkg-config --cflags --msvc-syntax " .. packages) }
    linkoptions { os.capture("pkg-config --libs --msvc-syntax " .. packages) }

  configuration { "gmake" }
    buildoptions { os.capture("pkg-config --cflags " .. packages) }
    linkoptions { os.capture("pkg-config --libs " .. packages) }
end

function io.readfile(source)
  local f = io.open(source, "r")
  local s = f:read("*a")
  f:close()
  return s
end

function io.writefile(dest, content)
  local f = io.open(dest, "w+")
  f:write(content)
  f:close()
end

function config_file(source, dest, dict)
  local s = io.readfile(source)
  s = python_format(s, dict)
  io.writefile(dest, s)
end
