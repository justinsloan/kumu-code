#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <vector>
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"

namespace {

// Prints a syntax error with the offending line underneath it and a caret at
// the exact spot -- far more use to a reader than a line number alone, which
// still leaves them counting lines in an editor.
// `appendLine` is for syntax errors, whose message doesn't carry its own line
// -- it is added here, after any clamping, so the number named always matches
// the line shown. Runtime messages already end with their own "(line N)".
void report(const std::string& kind, const std::string& message, int line, int column,
            const std::string& source, const std::string& trailer = "",
            bool appendLine = false) {
    auto withLine = [&](int n) {
        return message + (appendLine ? " (line " + std::to_string(n) + ")" : "");
    };
    if (line <= 0) {
        std::cerr << kind << ": " << message << trailer << std::endl;
        return;
    }

    std::vector<std::string> lines;
    std::istringstream stream(source);
    std::string text;
    while (std::getline(stream, text)) {
        if (!text.empty() && text.back() == '\r') text.pop_back();
        lines.push_back(text);
    }
    if (lines.empty()) {
        std::cerr << kind << ": " << withLine(line) << trailer << std::endl;
        return;
    }

    // An error at end of file sits one line past the last real one, so show
    // the last line instead and point just past its end -- which is exactly
    // where the missing piece belongs.
    int shown = line;
    int caret = column;
    if (shown > static_cast<int>(lines.size())) {
        shown = static_cast<int>(lines.size());
        caret = static_cast<int>(lines[shown - 1].size()) + 1;
    }
    const std::string& src = lines[shown - 1];

    // Reported after the clamp, so the line named always matches the line shown.
    std::cerr << kind << ": " << withLine(shown) << std::endl;

    const std::string gutter(std::to_string(shown).size(), ' ');
    std::cerr << '\n' << ' ' << shown << " | " << src << '\n'
              << ' ' << gutter << " | ";
    // Copy tabs from the source so the caret lands under the right character
    // whatever the terminal's tab width happens to be.
    for (int i = 0; i + 1 < caret && i < static_cast<int>(src.size()); ++i) {
        std::cerr << (src[i] == '\t' ? '\t' : ' ');
    }
    std::cerr << "^" << trailer << std::endl;
}

// A runtime message already ends with "(line N)"; a syntax one doesn't, so it
// gets the line appended here.
void reportSourceError(const kumu::SourceError& e, const std::string& source) {
    report("Syntax Error", e.what(), e.line, e.column, source, "", /*appendLine=*/true);
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename.kumu>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    // Read the entire file into a string
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    try {
        kumu::Lexer lexer(content);
        auto tokens = lexer.tokenize();

        kumu::Parser parser(tokens);
        auto program = parser.parse();

        kumu::Context ctx;
        kumu::Interpreter interpreter;

        interpreter.execute(program, ctx);
    } catch (const kumu::SourceError& e) {
        // Caught before the interpreter runs, so this is a problem with the
        // text of the program rather than with what it did.
        reportSourceError(e, content);
        return 1;
    } catch (const kumu::TracedError& e) {
        // An error that nothing caught: show the line with a caret on the
        // exact operator or name that failed, then the trail of calls that
        // led there.
        report("Runtime Error", e.what(), e.line, e.column, content, e.callStack);
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
