#include <CTcl.h>
#include <CArgs.h>
#include <CReadLine.h>

static std::string opts = "\
-debug:f \
";

static bool debug = false;

static CTcl      *tcl;
static CReadLine *readline;

bool
readLine(std::string &line)
{
  readline->setPrompt("% ");

  std::string line1 = readline->readLine();

  if (readline->eof()) return false;

  while (! tcl->isCompleteLine(line1)) {
    readline->setPrompt("");

    std::string line2 = readline->readLine();

    if (readline->eof()) return false;

    line1 += "\n" + line2;
  }

  line = line1;

  return true;
}

int
main(int argc, char **argv)
{
  CArgs cargs(opts);

  cargs.parse(&argc, argv);

  if (cargs.getBooleanArg("-debug"))
    debug = true;

  tcl = new CTcl(argc, argv);

  tcl->setDebug(debug);

  if (argc > 1)
    tcl->parseFile(argv[1]);
  else {
    readline = new CReadLine;

    std::string line;

    while (readLine(line)) {
      CTclValueRef value = tcl->parseLine(line);

      if (value.isValid()) {
        value->print(std::cout);

        std::cout << std::endl;
      }
    }
  }

  return 0;
}
