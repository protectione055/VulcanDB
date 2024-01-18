CFLAGS += -g
CXXFLAGS += -g

parser: express_parser_cli.o exp_parser_core.a
	$(CXX) $(CXXFLAGS) express_parser_cli.o exp_parser_core.a -o express_parser_cli

express_parser_cli.o: express_parser_cli.cpp express_parser.h
	$(CXX) $(CXXFLAGS) -c express_parser_cli.cpp -o express_parser_cli.o

exp_parser_core.a: express_parser.o yacc_exp.tab.o lex_exp.yy.o defs
	ar rcs exp_parser_core.a express_parser.o yacc_exp.tab.o lex_exp.yy.o express_defs.o

express_parser.o: express_parser.cpp yacc express_defs.h
	$(CXX) $(CXXFLAGS) -c express_parser.cpp -o express_parser.o

lex_exp.yy.o: lex yacc express_defs.h
	$(CC) $(CFLAGS) -c lex.yy.c -o lex_exp.yy.o

yacc_exp.tab.o: lex yacc express_defs.h
	$(CC) $(CFLAGS) -c yacc_exp.tab.c -o yacc_exp.tab.o

defs: express_defs.h express_defs.c
	$(CC) $(CFLAGS) -c express_defs.c -o express_defs.o
	
lex: lex_exp.l
	flex  --header-file=lex_exp.yy.h lex_exp.l

yacc: yacc_exp.y
	# bison -d --debug --verbose -b yacc_exp yacc_exp.y
	bison -d -b yacc_exp yacc_exp.y

clean:
	rm -f *.o lex_exp.yy.* yacc_exp.tab.* express_parser lex.yy.c yacc_exp.output