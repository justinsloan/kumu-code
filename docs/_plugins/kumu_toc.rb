# frozen_string_literal: true

# Builds the "On this page" list for the long single-page documents.
#
# jekyll-toc returned nothing against just-the-docs' heading markup (each
# heading carries an anchor <svg> before its text), so this does the job
# directly: scan the rendered HTML for h2s, strip the anchor decoration,
# emit a flat nav list. h2 is exactly the right level -- the textbook's
# h2s are its 16 chapters, the manual's are its 10 numbered sections.

module Jekyll
  module KumuTocFilter
    HEADING = %r{<h2[^>]*\sid="([^"]+)"[^>]*>(.*?)</h2>}m

    def kumu_toc(html)
      items = html.to_s.scan(HEADING).filter_map do |id, inner|
        text = inner
               .gsub(%r{<svg.*?</svg>}m, "")
               .gsub(/<[^>]+>/, "")
               .gsub(/\s+/, " ")
               .strip
        next if text.empty?

        %(<li class="kumu-toc__item"><a href="##{id}">#{text}</a></li>)
      end

      return "" if items.empty?

      %(<ul class="kumu-toc__list">#{items.join}</ul>)
    end
  end
end

Liquid::Template.register_filter(Jekyll::KumuTocFilter)
