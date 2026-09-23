# frozen_string_literal: true

# A Rouge lexer for Kumu.
#
# The docs use two info strings: ```kumu for ordinary examples and
# ```kumu-broken for the deliberately-wrong programs in the textbook's
# "Common mistakes" sections. Rouge knows neither, so without this every
# code block on the site would render unhighlighted.

require "rouge"

module Rouge
  module Lexers
    class Kumu < RegexLexer
      title "Kumu"
      desc "The Kumu teaching language"
      tag "kumu"
      aliases "kumu-broken"
      filenames "*.kumu"

      KEYWORDS = %w[
        VAR PRINT IF ELSEIF ELSE WHILE FOR EACH IN TO
        FUNCTION RETURN TRY CATCH TRACE INPUT
        CLEAR_SCREEN SLEEP
      ].freeze

      OPERATOR_WORDS = %w[AND OR NOT MOD].freeze

      BUILTINS = %w[
        LEN SUBSTRING INDEX_OF UPPER LOWER TRIM REPLACE CONTAINS
        STARTS_WITH ENDS_WITH SPLIT RANDOM
        READ_FILE WRITE_FILE READ_CSV FILE_EXISTS
      ].freeze

      state :root do
        rule %r/#.*/, Comment::Single

        # Kumu strings have no escape sequences and may span lines.
        rule %r/"[^"]*"/m, Str::Double

        rule %r/\b\d+(\.\d+)?\b/, Num

        # Dot-methods: .GET, .APPEND_ROW, .SORT, ...
        rule %r/\.[A-Z][A-Z_]*/, Name::Function

        rule %r/\b(?:#{KEYWORDS.join('|')})\b/o,       Keyword
        rule %r/\b(?:#{OPERATOR_WORDS.join('|')})\b/o, Operator::Word
        rule %r/\b(?:#{BUILTINS.join('|')})\b/o,       Name::Builtin

        rule %r/[A-Za-z_][A-Za-z0-9_]*/, Name
        rule %r/[=!<>]=|[-+*\/<>=!]/,    Operator
        rule %r/[(){}\[\],]/,            Punctuation
        rule %r/\s+/m,                   Text
        rule %r/./,                      Text
      end
    end
  end
end
