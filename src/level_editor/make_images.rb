source, target = ARGV[0], ARGV[1]

images = []

if (RUBY_VERSION.split(".").map{|s|s.to_i} <=> [1,9,0]) < 0
  def ord(s) s[0] end
else
  def ord(s) s.ord end
end

File.open(target, 'w') do |dst|
  dst.puts(<<-END_OF_HEADER)
#include "image_data.hpp"

namespace Graal {
  namespace level_editor {
    namespace image_data {
  END_OF_HEADER
  Dir.glob(File.join(source, '*.png')) do |img_name|
    path = img_name
    img_name = File.basename(img_name)
    next if File.directory?(path)
    File.open(path, "rb") do |img|
      var_name = img_name[0...-4].gsub(/\W+/, '_')
      images << var_name
      if /^\d/.match(var_name)
        var_name.insert(0, '_')
      end
      dst.print("      const char #{var_name}[] =")
      while data = img.read(68)
        dst.print "\n        \"#{data.gsub(/[^[:print:]]|\\|"/) { |ch|
          "\\#{ord(ch).to_s(8).rjust(3, '0')}"
        }}\""
      end
      dst.print(";\n\n")
    end
  end
  dst.puts "      const char* images[] = {"
  images.each do |img|
    dst.puts "        \"internal/#{img}.png\", #{img}, #{img} + sizeof(#{img}) - 1,"
  end
  dst.puts(<<-END_IMAGES)
        0, 0, 0
      };
  END_IMAGES

  dst.puts(<<-END_OF_FOOTER)
    }
  }
}
  END_OF_FOOTER
end

target = target.sub(/\.cpp$/, '.hpp')

File.open(target, 'w') do |dst|
  guard_token = "GRAAL_LEVEL_EDITOR_#{target.gsub(/\W+/, '_').upcase}_"
  dst.puts(<<-END_OF_HEADER)
#ifndef #{guard_token}
#define #{guard_token}
namespace Graal {
  namespace level_editor {
    namespace image_data {
  END_OF_HEADER
  images.each do |var_name|
    dst.puts "      extern const char #{var_name}[];"
  end
  dst.puts(<<-END_OF_FOOTER)
      extern const char* images[];
    }
  }
}
#endif
  END_OF_FOOTER
end
