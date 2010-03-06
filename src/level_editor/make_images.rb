source, target = ARGV[0], ARGV[1]

images = []

File.open(target, 'w') do |dst|
  dst.puts(<<-END_OF_HEADER)
namespace Graal {
  namespace level_editor {
    namespace image_data {
  END_OF_HEADER
  Dir.glob(source) do |img_name|
    #path = File.join(source, img_name)
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
          "\\#{ch[0].to_s(8).rjust(3, '0')}"
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
