# include <iostream>
# include <string>
# include <cstring>
# include <vector>
# include <cstdlib>
# include <stdio.h>
# include <stdexcept>
# include <iomanip>
using namespace std;

class Token {
  public :
  int m_line ;
  int m_column ;
  string m_type ;
  string m_content ;
  int m_string_size ;
  Token() {
    m_line = -1 ;
    m_column = -1 ;
    m_type = "" ;
    m_content = "" ;
    m_string_size = 0 ;
  } // Token()
} ;

class Node {
  public:
  Node* m_left ;
  Node* m_right ;
  Token m_token ;
  bool m_start ;
  Node() {
    Token token ;
    m_left = NULL ;
    m_right = NULL;
    m_token = token ;
    m_start = false ;
  } // Node()
};

class Symbol {
  public:
  string m_name ;
  bool m_overwrite ;
  Node* m_head ;
  Symbol( string name, bool overwrite, Node* head ) {
    m_name = name ;
    m_overwrite = overwrite ;
    if ( m_overwrite )
      m_head = head ;
    else {
      m_head = new Node() ;
      m_head->m_token.m_type = "SYMBOL" ;
      m_head->m_token.m_content = "#<procedure " + m_name + ">" ;
    } // else
  } // Symbol()
};

class Our_Exception {
  public:
  string m_msg ;
  Node* m_head ;
  Our_Exception() {
    m_msg = "Why would you throw an empty exception?";
  } // Our_Exception()

  Our_Exception( string msg, Node* head ) {
    m_msg = msg ;
    m_head = head ;
  } // Our_Exception()
};

int g_line = 0 ;
int g_column = 0 ;
int g_print = 0 ;
vector<Token> g_TokenBuffer ;
vector<Token> g_TokenList ;
vector<Symbol>g_SymbolList ;

Node* Tree_S_exp() ;
Node* Tree_Atom() ;
Node* Tree_Contigious_S_exp() ;
void Pretty_Print( Node* head ) ;
void Print_Start( Node* head ) ;
void Print_Left( Node* head ) ;
void Print_Right( Node* head ) ;

// ------------------------------ Global Boolean ------------------------------
string To_string( int i, float f, string type ) {
  char temp[50] ;
  string result ;
  if ( type == "i" )
    sprintf( temp, "%d", i ) ;
  else if ( type == "f" )
    sprintf( temp, "%.3f", f ) ;
  result = string( temp ) ;
  return result ;
} // To_string()

string Token_type_string( Node* node ) {
  if ( node->m_token.m_type == "NIL" )
    return "nil" ;
  else if ( node->m_token.m_type == "T" )
    return "#t" ;
  else if ( node->m_token.m_type == "FLOAT" )
    return To_string( 0, atof( node->m_token.m_content.c_str() ), "f" ) ;
  else if ( node->m_token.m_type == "INT" )
    return To_string( atoi( node->m_token.m_content.c_str() ), 0, "i" ) ;
  else if ( node->m_token.m_type == "QUOTE" )
    return "quote" ;
  else if ( node->m_token.m_content == "#<procedure +>" )
    return "+" ;
  else if ( node->m_token.m_content == "#<procedure ->" )
    return "-" ;
  else if ( node->m_token.m_content == "#<procedure *>" )
    return "*" ;
  else if ( node->m_token.m_content == "#<procedure />" )
    return "/" ;
  else if ( node->m_token.m_content == "#<procedure and>" )
    return "+" ;
  else if ( node->m_token.m_content == "#<procedure or>" )
    return "+" ;
  else if ( node->m_token.m_content == "#<procedure >>" )
    return ">" ;
  else if ( node->m_token.m_content == "#<procedure >=>" )
    return ">=" ;
  else if ( node->m_token.m_content == "#<procedure <>" )
    return "<" ;
  else if ( node->m_token.m_content == "#<procedure <=>" )
    return "<=" ;
  else if ( node->m_token.m_content == "#<procedure =>" )
    return "=" ;
  else if ( node->m_token.m_content == "#<procedure string>?>" )
    return "string>?" ;
  else if ( node->m_token.m_content == "#<procedure string<?>" )
    return "string<?" ;
  else if ( node->m_token.m_content == "#<procedure string=?>" )
    return "string=?" ;
  else
    return node->m_token.m_content ;
} // Token_type_string()

bool Is_Atom( Node* head ) {
  if ( head->m_token.m_type == "SYMBOL" || head->m_token.m_type == "INT" ||
       head->m_token.m_type == "FLOAT" ||  head->m_token.m_type == "STRING" ||
       head->m_token.m_type == "NIL" || head->m_token.m_type == "T" )
    return true ;
  return false ;
} // Is_Atom()

bool Is_digit( char temp ) {
  if ( temp >= '0' && temp <= '9' )  return true ;
  return false ;
} // Is_digit()

bool Is_ws( char temp ) {
  if ( temp == ' ' || temp == '\t' || temp == '\n' )
    return true ;
  return false ;
} // Is_ws()

bool Is_int( string temp ) {
  int num_count = 0 ;
  int i = 0 ;
  if ( temp[i] == '-' || temp[i] == '+' ) i++ ;

  for ( ; i < temp.size() ; i++ ) {
    if ( Is_digit( temp[i] ) ) num_count++ ;
    if ( !Is_digit( temp[i] ) ) return false ;
  } // for

  if ( num_count == 0 ) return false ;
  return true ;
} // Is_int()

bool Is_float( string temp ) {
  int i = 0 ;
  int dot_count = 0 ;
  int num_count = 0 ;
  if ( temp[i] == '-' || temp[i] == '+' ) i++ ;

  for ( ; i < temp.size() ; i++ ) {
    if ( temp[i] == '.' ) dot_count++ ;
    if ( Is_digit( temp[i] ) ) num_count++ ;
    if ( ( !Is_digit( temp[i] ) && temp[i] != '.' ) || dot_count > 1 ) return false ;
  } // for

  if ( num_count == 0 ) return false ;
  return true ;
} // Is_float()

// ------------------------------ Get Token ------------------------------
string Get_type( string content ) {
  string type ;
  if ( content == "(" )  type = "LEFT-PAREN" ;
  else if ( content == ")" )  type = "RIGHT-PAREN" ;
  else if ( Is_int( content ) ) type = "INT" ;
  else if ( Is_float( content ) ) type = "FLOAT" ;
  else if ( content[0] == '\"' ) {
    if ( content[content.size()-1] == '\"' && content.size() > 1 ) type = "STRING" ;
    else type = "UNKNOWN" ;
  } // else if
  else if ( content == "." )  type = "DOT" ;
  else if ( content == "nil" || content == "#f" || content == "()" )  type = "NIL" ;
  else if ( content == "t" || content == "#t" )  type = "T" ;
  else if ( content == "\'" )  type = "QUOTE" ;
  else type = "SYMBOL" ;
  return type ;
} // Get_type()

char Get_next_char() {
  char temp = getchar() ;
  if ( temp == '\n' ) {
    g_line++ ;
    g_column = 0 ;
  } // if
  else {
    g_column++ ;
  } // else

  return temp ;
} // Get_next_char()

char Get_next_non_ws_char() {
  char temp = Get_next_char() ;
  while ( Is_ws( temp ) ) {
    temp = Get_next_char() ;
  } // while

  return temp ;
} // Get_next_non_ws_char()

Token Get_next_token() {
  Token token ;
  char first_char = Get_next_non_ws_char() ;
  token.m_line = g_line ;
  token.m_column = g_column ;
  if ( first_char == EOF )
    return token ;
  else if ( first_char == ';' ) {
    while ( first_char != '\n' && first_char != EOF )
      first_char = Get_next_char() ;
    return Get_next_token() ;
  } // else if

  char next_char = cin.peek() ;
  bool found = false ;

  token.m_content += first_char ;
  if ( first_char == '(' || first_char == ')' || ( first_char == '.' && next_char == '\0' ) ||
       first_char == '\'' ) {
    token.m_type = Get_type( token.m_content ) ;
    return token ;
  } // if
  else if ( first_char == '\"' ) {
    token.m_string_size++ ;
    while ( next_char != '\"' ) {
      if ( next_char == '\\' ) {
        Get_next_char() ;
        next_char = cin.peek() ;
        if ( next_char == '\"' ) {
          token.m_content += '\"' ;
          next_char = '*' ;
        } // if
        else if ( next_char == 'n' )
          token.m_content += '\n' ;
        else if ( next_char == 't' )
          token.m_content += '\t' ;
        else if ( next_char == '\\' )
          token.m_content += '\\' ;
        else
          token.m_content = token.m_content + "\\" + next_char ;
        token.m_string_size += 2 ;
      } // if
      else if ( next_char == '\n' || next_char == EOF ) {
        token.m_type = "UNKNOWN" ;
        return token ;
      } // else if
      else {
        token.m_content += next_char ;
        token.m_string_size++ ;
      } // else

      Get_next_char() ;
      next_char = cin.peek() ;
    } // while

    token.m_string_size++ ;
    token.m_content += Get_next_char() ;
  } // else if
  else {
    while ( !Is_ws( next_char ) && next_char != '(' && next_char != ')' && next_char != '\'' &&
            next_char != '\"' && next_char != ';' ) {
      token.m_content += next_char ;
      Get_next_char() ;
      next_char = cin.peek() ;
    } // while
  } // else

  token.m_type = Get_type( token.m_content ) ;
  return token ;
} // Get_next_token()

Token Retrieve_token() {
  Token token ;
  if ( g_TokenBuffer.size() >= 1 ) {
    token = g_TokenBuffer[0] ;
    g_TokenBuffer.erase( g_TokenBuffer.begin() ) ;
  } // if
  else
    token = Get_next_token() ;

  return token ;
} // Retrieve_token()

// ------------------------------ Syntax ------------------------------
void Atom( Token &token ) {
  if ( token.m_type == "UNKNOWN" )
    throw new Our_Exception( "ERROR (no closing quote) : END-OF-LINE encountered at", NULL ) ;
  if ( token.m_type != "SYMBOL" && token.m_type != "INT" &&
       token.m_type != "FLOAT" &&  token.m_type != "STRING" &&
       token.m_type != "NIL" && token.m_type != "T" )
    throw new Our_Exception( "ERROR (unexpected token) : atom or '(' expected when token at", NULL ) ;

  g_TokenList.push_back( token ) ;
} // Atom()

void S_exp( Token &token ) {
  if ( token.m_type == "LEFT-PAREN" ) {
    Token next_token = Retrieve_token() ;
    if ( next_token.m_type == "RIGHT-PAREN" ) {
      token.m_content = "()" ;
      token.m_type = "NIL" ;
    } // if
    else
      g_TokenBuffer.push_back( next_token ) ;
  } // if

  if ( token.m_type == "LEFT-PAREN" ) {
    g_TokenList.push_back( token ) ;
    token = Retrieve_token() ;
    S_exp( token ) ;
    token = Retrieve_token() ;
    while ( token.m_type == "LEFT-PAREN" || token.m_type == "QUOTE" ||
            token.m_type == "SYMBOL" || token.m_type == "INT" ||
            token.m_type == "FLOAT" || token.m_type == "STRING" ||
            token.m_type == "NIL" || token.m_type == "T" ) {
      S_exp( token ) ;
      token = Retrieve_token() ;
    } // while

    if ( token.m_type == "UNKNOWN" )
      throw new Our_Exception( "ERROR (no closing quote) : END-OF-LINE encountered at", NULL ) ;

    if ( token.m_type == "DOT" ) {
      g_TokenList.push_back( token ) ;
      token = Retrieve_token() ;
      S_exp( token ) ;
      token = Retrieve_token() ;
    } // if

    if ( token.m_type != "RIGHT-PAREN" )
      throw new Our_Exception( "ERROR (unexpected token) : ')' expected when token at", NULL ) ;

    g_TokenList.push_back( token ) ;
  } // if
  else if ( token.m_type == "QUOTE" ) {
    g_TokenList.push_back( token ) ;
    token = Retrieve_token() ;
    S_exp( token ) ;
  } // else if
  else {
    Atom( token ) ;
  } // else
} // S_exp()

// ------------------------------ Create Tree ------------------------------
Node* Tree_Atom() {
  Node* node = new Node ;
  node->m_left = NULL ;
  node->m_right = NULL ;
  node->m_token = g_TokenList[0] ;
  g_TokenList.erase( g_TokenList.begin() ) ;
  return node ;
} // Tree_Atom()

Node* Tree_S_exp() {
  Node* head = new Node() ;
  Token token = g_TokenList[0] ;
  if ( token.m_type == "LEFT-PAREN" ) {
    g_TokenList.erase( g_TokenList.begin() ) ;
    head->m_left = Tree_S_exp() ;
    token = g_TokenList[0] ;

    if ( token.m_type == "LEFT-PAREN" || token.m_type == "QUOTE" ||
         token.m_type == "SYMBOL" || token.m_type == "INT" ||
         token.m_type == "FLOAT" || token.m_type == "STRING" ||
         token.m_type == "NIL" || token.m_type == "T" ||
         token.m_type == "DOT" ) {
      head->m_right = Tree_Contigious_S_exp() ;
    } // if
    else {
      Node* node = new Node() ;
      node->m_token.m_type = "NIL" ;
      node->m_token.m_content = "nil" ;
      head->m_right = node ;
    } // else

    g_TokenList.erase( g_TokenList.begin() ) ;
  } // if
  else if ( token.m_type == "QUOTE" ) {
    // remove token "'"
    // create new "quote" node
    // head's left is quote node
    // head's right is "new empty node"
    // "new empty node" left is "S_Exp"
    // "new empty node" right is "nil" node
    g_TokenList.erase( g_TokenList.begin() ) ;
    Node* node_quote = new Node() ;
    node_quote->m_token.m_content = "quote" ;
    // node_quote->m_token.m_type = "QUOTE" ;
    node_quote->m_token.m_type = "SYMBOL" ;
    head->m_left = node_quote ;
    Node* node_right = new Node() ;
    node_right->m_left = Tree_S_exp() ;
    Node* node = new Node() ;
    node->m_token.m_type = "NIL" ;
    node->m_token.m_content = "nil" ;
    head->m_right = node ;
    node_right->m_right = node ;
    head->m_right = node_right ;
  } // else if
  else {
    head = Tree_Atom() ;
  } // else

  return head ;
} // Tree_S_exp()

Node* Tree_Contigious_S_exp() {
  if ( g_TokenList[0].m_type == "RIGHT-PAREN" ) {
    Node* node = new Node() ;
    node->m_token.m_type = "NIL" ;
    node->m_token.m_content = "nil" ;
    return node ;
    // return NULL ;
  } // if
  else if ( g_TokenList[0].m_type == "DOT" ) {
    g_TokenList.erase( g_TokenList.begin() ) ;
    Node* node = Tree_S_exp() ;
    return node ;
  } // if
  else {
    Node* node = new Node() ;
    node->m_left = Tree_S_exp() ;
    node->m_right = Tree_Contigious_S_exp() ;
    return node ;
  } // else
} // Tree_Contigious_S_exp()

// ------------------------------ Print ------------------------------
void Print_Space() {
  for ( int i = 0 ; i < g_print ; i++ )
    cout << " " ;
} // Print_Space()

void Pretty_Print( Node* head ) {
  if ( head == NULL ) return ;
  else {
    if ( head->m_token.m_type == "NIL" )  cout << "nil" << endl ;
    else if ( head->m_token.m_type == "T" )  cout << "#t" << endl ;
    else if ( head->m_token.m_type == "FLOAT" )
      printf( "%.3f\n", atof( head->m_token.m_content.c_str() ) );
    else if ( head->m_token.m_type == "INT" )
      printf( "%d\n", atoi( head->m_token.m_content.c_str() ) );
    // else if ( head->m_token.m_type == "QUOTE" ) cout << "quote" << endl ;
    else
      cout << head->m_token.m_content << endl ;
  } // else
} // Pretty_Print()

void Print_Left( Node* head ) {
  if ( head == NULL ) return ;
  else if ( Is_Atom( head ) || head->m_token.m_type == "QUOTE" )
    Pretty_Print( head ) ;
  else
    Print_Start( head ) ;
} // Print_Left()

void Print_Right( Node* head ) {
  if ( head == NULL ) return ;
  else if ( Is_Atom( head ) && head->m_token.m_type != "NIL" ) {
    Print_Space() ;
    cout << "." << endl ;
    Print_Space() ;
    Pretty_Print( head ) ;
  } // else if
  else {
    if ( head->m_left != NULL && head->m_left->m_token.m_content == "" )
      Print_Space() ;
    Print_Start( head->m_left ) ;
    Print_Right( head->m_right ) ;
  } // else
} // Print_Right()

void Print_Start( Node* head ) {
  if ( head == NULL ) return ;
  else if ( Is_Atom( head ) ) {
    Print_Space() ;
    Pretty_Print( head ) ;
  } // else if
  else {
    cout << "( " ;
    g_print += 2 ;
    Print_Left( head->m_left ) ;
    Print_Right( head->m_right ) ;
    g_print -= 2 ;
    Print_Space() ;
    cout << ")" << endl ;
  } // else
} // Print_Start()


// ------------------------------ Initialize Symbols ------------------------------
void Initialize_Symbols() {
  g_SymbolList.clear() ;
  Symbol cons( "cons", false, NULL ) ;
  Symbol list( "list", false, NULL ) ;
  Symbol quote( "quote", false, NULL ) ;
  Symbol define( "define", false, NULL ) ;
  Symbol car( "car", false, NULL ) ;
  Symbol cdr( "cdr", false, NULL ) ;
  Symbol b_atom( "atom?", false, NULL ) ;
  Symbol b_list( "list?", false, NULL ) ;
  Symbol b_pair( "pair?", false, NULL ) ;
  Symbol b_null( "null?", false, NULL ) ;
  Symbol b_integer( "integer?", false, NULL ) ;
  Symbol b_real( "real?", false, NULL ) ;
  Symbol b_number( "number?", false, NULL ) ;
  Symbol b_string( "string?", false, NULL ) ;
  Symbol b_boolean( "boolean?", false, NULL ) ;
  Symbol b_symbol( "symbol?", false, NULL ) ;
  Symbol add( "+", false, NULL ) ;
  Symbol sub( "-", false, NULL ) ;
  Symbol mul( "*", false, NULL ) ;
  Symbol div( "/", false, NULL ) ;
  Symbol and_gate( "and", false, NULL ) ;
  Symbol or_gate( "or", false, NULL ) ;
  Symbol non( "not", false, NULL ) ;
  Symbol gt( ">", false, NULL ) ;
  Symbol ge( ">=", false, NULL ) ;
  Symbol st( "<", false, NULL ) ;
  Symbol se( "<=", false, NULL ) ;
  Symbol eq( "=", false, NULL ) ;
  Symbol str_append( "string-append", false, NULL ) ;
  Symbol str_gt( "string>?", false, NULL ) ;
  Symbol str_st( "string<?", false, NULL ) ;
  Symbol str_eq( "string=?", false, NULL ) ;
  Symbol eqv( "eqv?", false, NULL ) ;
  Symbol equality( "equal?", false, NULL ) ;
  Symbol beg( "begin", false, NULL ) ;
  Symbol cond( "cond", false, NULL ) ;
  Symbol if_cond( "if", false, NULL ) ;
  Symbol clean( "clean-environment", false, NULL ) ;
  Symbol exit( "exit", false, NULL ) ;
  g_SymbolList.push_back( cons ) ;
  g_SymbolList.push_back( list ) ;
  g_SymbolList.push_back( quote ) ;
  g_SymbolList.push_back( define ) ;
  g_SymbolList.push_back( car ) ;
  g_SymbolList.push_back( cdr ) ;
  g_SymbolList.push_back( b_atom ) ;
  g_SymbolList.push_back( b_list ) ;
  g_SymbolList.push_back( b_pair ) ;
  g_SymbolList.push_back( b_list ) ;
  g_SymbolList.push_back( b_null ) ;
  g_SymbolList.push_back( b_integer ) ;
  g_SymbolList.push_back( b_real ) ;
  g_SymbolList.push_back( b_number ) ;
  g_SymbolList.push_back( b_string ) ;
  g_SymbolList.push_back( b_boolean ) ;
  g_SymbolList.push_back( b_symbol ) ;
  g_SymbolList.push_back( add ) ;
  g_SymbolList.push_back( sub ) ;
  g_SymbolList.push_back( mul ) ;
  g_SymbolList.push_back( div ) ;
  g_SymbolList.push_back( and_gate ) ;
  g_SymbolList.push_back( or_gate ) ;
  g_SymbolList.push_back( non ) ;
  g_SymbolList.push_back( gt ) ;
  g_SymbolList.push_back( ge ) ;
  g_SymbolList.push_back( st ) ;
  g_SymbolList.push_back( se ) ;
  g_SymbolList.push_back( eq ) ;
  g_SymbolList.push_back( str_append ) ;
  g_SymbolList.push_back( str_gt ) ;
  g_SymbolList.push_back( str_st ) ;
  g_SymbolList.push_back( str_eq ) ;
  g_SymbolList.push_back( eqv ) ;
  g_SymbolList.push_back( equality ) ;
  g_SymbolList.push_back( beg ) ;
  g_SymbolList.push_back( cond ) ;
  g_SymbolList.push_back( if_cond ) ;
  g_SymbolList.push_back( clean ) ;
  g_SymbolList.push_back( exit ) ;
} // Initialize_Symbols()


// ------------------------------ Execution ------------------------------
bool Test_Equal( Node* head1, Node* head2 ) {
  if ( head1 == NULL || head2 == NULL ) {
    if ( ( head1 == NULL && head2 != NULL ) || ( head1 != NULL && head2 == NULL ) )
      return false ;
    return true ;
  } // if
  else {
    if ( head1->m_token.m_content != head2->m_token.m_content )
      return false ;
    if ( !Test_Equal( head1->m_left, head2->m_left ) )
      return false ;
    if ( !Test_Equal( head1->m_right, head2->m_right ) )
      return false ;
    return true ;
  } // else
} // Test_Equal()

Node* Execute_Arithmetic( Node* arg1, Node* arg2, string op ) {
  Node* node = new Node() ;
  if ( op == "+" ) {
    if ( arg1->m_token.m_type == "INT" && arg2->m_token.m_type == "FLOAT" ) {
      node->m_token.m_type = "FLOAT" ;
      node->m_token.m_content = To_string( 0, atoi( arg1->m_token.m_content.c_str() ) +
                                           atof( arg2->m_token.m_content.c_str() ), "f" ) ;
    } // if
    else if ( arg1->m_token.m_type == "FLOAT" && arg2->m_token.m_type == "INT" ) {
      node->m_token.m_type = "FLOAT" ;
      node->m_token.m_content = To_string( 0, atof( arg1->m_token.m_content.c_str() ) +
                                           atoi( arg2->m_token.m_content.c_str() ), "f" ) ;
    } // else if
    else if ( arg1->m_token.m_type == "INT" && arg2->m_token.m_type == "INT" ) {
      node->m_token.m_type = "INT" ;
      node->m_token.m_content = To_string( atoi( arg1->m_token.m_content.c_str() ) +
                                           atoi( arg2->m_token.m_content.c_str() ), 0, "i" ) ;
    } // else if
    else if ( arg1->m_token.m_type == "FLOAT" && arg2->m_token.m_type == "FLOAT" ) {
      node->m_token.m_type = "FLOAT" ;
      node->m_token.m_content = To_string( 0, atof( arg1->m_token.m_content.c_str() ) +
                                           atof( arg2->m_token.m_content.c_str() ), "f" ) ;
    } // else if
    else {
      if ( arg1->m_token.m_type != "INT" && arg1->m_token.m_type != "FLOAT" ) {
        throw new Our_Exception( "ERROR (+ with incorrect argument type) : ", arg1 ) ;
      } // if
      else {
        throw new Our_Exception( "ERROR (+ with incorrect argument type) : ", arg2 ) ;
      } // else
    } // else

  } // if
  else if ( op == "-" ) {
    if ( arg1->m_token.m_type == "INT" && arg2->m_token.m_type == "FLOAT" ) {
      node->m_token.m_type = "FLOAT" ;
      node->m_token.m_content = To_string( 0, atoi( arg1->m_token.m_content.c_str() ) -
                                           atof( arg2->m_token.m_content.c_str() ), "f" ) ;
    } // if
    else if ( arg1->m_token.m_type == "FLOAT" && arg2->m_token.m_type == "INT" ) {
      node->m_token.m_type = "FLOAT" ;
      node->m_token.m_content = To_string( 0, atof( arg1->m_token.m_content.c_str() ) -
                                           atoi( arg2->m_token.m_content.c_str() ), "f" ) ;
    } // else if
    else if ( arg1->m_token.m_type == "INT" && arg2->m_token.m_type == "INT" ) {
      node->m_token.m_type = "INT" ;
      node->m_token.m_content = To_string( atoi( arg1->m_token.m_content.c_str() ) -
                                           atoi( arg2->m_token.m_content.c_str() ), 0, "i" ) ;
    } // else if
    else if ( arg1->m_token.m_type == "FLOAT" && arg2->m_token.m_type == "FLOAT" ) {
      node->m_token.m_type = "FLOAT" ;
      node->m_token.m_content = To_string( 0, atof( arg1->m_token.m_content.c_str() ) -
                                           atof( arg2->m_token.m_content.c_str() ), "f" ) ;
    } // else if
    else {
      if ( arg1->m_token.m_type != "INT" && arg1->m_token.m_type != "FLOAT" ) {
        throw new Our_Exception( "ERROR (- with incorrect argument type) : ", arg1 ) ;
      } // if
      else {
        throw new Our_Exception( "ERROR (- with incorrect argument type) : ", arg2 ) ;
      } // else
    } // else
  } // else if
  else if ( op == "*" ) {
    if ( arg1->m_token.m_type == "INT" && arg2->m_token.m_type == "FLOAT" ) {
      node->m_token.m_type = "FLOAT" ;
      node->m_token.m_content = To_string( 0, atoi( arg1->m_token.m_content.c_str() ) *
                                           atof( arg2->m_token.m_content.c_str() ), "f" ) ;
    } // if
    else if ( arg1->m_token.m_type == "FLOAT" && arg2->m_token.m_type == "INT" ) {
      node->m_token.m_type = "FLOAT" ;
      node->m_token.m_content = To_string( 0, atof( arg1->m_token.m_content.c_str() ) *
                                           atoi( arg2->m_token.m_content.c_str() ), "f" ) ;
    } // else if
    else if ( arg1->m_token.m_type == "INT" && arg2->m_token.m_type == "INT" ) {
      node->m_token.m_type = "INT" ;
      node->m_token.m_content = To_string( atoi( arg1->m_token.m_content.c_str() ) *
                                           atoi( arg2->m_token.m_content.c_str() ), 0, "i" ) ;
    } // else if
    else if ( arg1->m_token.m_type == "FLOAT" && arg2->m_token.m_type == "FLOAT" ) {
      node->m_token.m_type = "FLOAT" ;
      node->m_token.m_content = To_string( 0, atof( arg1->m_token.m_content.c_str() ) *
                                           atof( arg2->m_token.m_content.c_str() ), "f" ) ;
    } // else if
    else {
      if ( arg1->m_token.m_type != "INT" && arg1->m_token.m_type != "FLOAT" ) {
        throw new Our_Exception( "ERROR (* with incorrect argument type) : ", arg1 ) ;
      } // if
      else {
        throw new Our_Exception( "ERROR (* with incorrect argument type) : ", arg2 ) ;
      } // else
    } // else
  } // else if
  else if ( op == "/" ) {
    if ( arg2->m_token.m_content == "0" )
      throw new Our_Exception( "ERROR (division by zero) : /", NULL ) ;
    if ( arg1->m_token.m_type == "INT" && arg2->m_token.m_type == "FLOAT" ) {
      node->m_token.m_type = "FLOAT" ;
      node->m_token.m_content = To_string( 0, atoi( arg1->m_token.m_content.c_str() ) /
                                           atof( arg2->m_token.m_content.c_str() ), "f" ) ;
    } // if
    else if ( arg1->m_token.m_type == "FLOAT" && arg2->m_token.m_type == "INT" ) {
      node->m_token.m_type = "FLOAT" ;
      node->m_token.m_content = To_string( 0, atof( arg1->m_token.m_content.c_str() ) /
                                           atoi( arg2->m_token.m_content.c_str() ), "f" ) ;
    } // else if
    else if ( arg1->m_token.m_type == "INT" && arg2->m_token.m_type == "INT" ) {
      node->m_token.m_type = "INT" ;
      node->m_token.m_content = To_string( atoi( arg1->m_token.m_content.c_str() ) /
                                           atoi( arg2->m_token.m_content.c_str() ), 0, "i" ) ;
    } // else if
    else if ( arg1->m_token.m_type == "FLOAT" && arg2->m_token.m_type == "FLOAT" ) {
      node->m_token.m_type = "FLOAT" ;
      node->m_token.m_content = To_string( 0, atof( arg1->m_token.m_content.c_str() ) /
                                           atof( arg2->m_token.m_content.c_str() ), "f" ) ;
    } // else if
    else {
      if ( arg1->m_token.m_type != "INT" && arg1->m_token.m_type != "FLOAT" ) {
        throw new Our_Exception( "ERROR (/ with incorrect argument type) : ", arg1 ) ;
      } // if
      else {
        string type = Token_type_string( arg2 ) ;
        throw new Our_Exception( "ERROR (/ with incorrect argument type) : ", arg2 ) ;
      } // else
    } // else
  } // else if
  else if ( op == "not" ) {
    if ( arg1->m_token.m_type == "NIL" ) {
      node->m_token.m_type = "T" ;
      node->m_token.m_content = "#t" ;
    } // if
    else if ( arg1->m_token.m_type == "T" ) {
      node->m_token.m_type = "NIL" ;
      node->m_token.m_content = "nil" ;
    } // if
    else {
      node->m_token.m_type = "NIL" ;
      node->m_token.m_content = "nil" ;
    } // else
  } // else if
  else if ( op == "and" ) {
    if ( arg1->m_token.m_type == "NIL" || arg2->m_token.m_type == "NIL" ) {
      node->m_token.m_type = "NIL" ;
      node->m_token.m_content = "nil" ;
    } // if
    else
      node = arg2 ;
  } // else if
  else if ( op == "or" ) {
    if ( arg1->m_token.m_type != "NIL" )
      node = arg1 ;
    else
      node = arg2 ;
  } // else if
  else if ( op == ">" ) {
    float num1, num2 ;
    if ( arg1->m_token.m_type == "INT" )
      num1 = atoi( arg1->m_token.m_content.c_str() ) ;
    else if ( arg1->m_token.m_type == "FLOAT" )
      num1 = atof( arg1->m_token.m_content.c_str() ) ;
    else {
      throw new Our_Exception( "ERROR (> with incorrect argument type) : ", arg1 ) ;
    } // else

    if ( arg2->m_token.m_type == "INT" )
      num2 = atoi( arg2->m_token.m_content.c_str() ) ;
    else if ( arg2->m_token.m_type == "FLOAT" )
      num2 = atof( arg2->m_token.m_content.c_str() ) ;
    else {
      throw new Our_Exception( "ERROR (> with incorrect argument type) : ", arg2 ) ;
    } // else

    if ( num1 > num2 ) {
      node->m_token.m_type = "T" ;
      node->m_token.m_content = "#t" ;
    } // if
    else {
      node->m_token.m_type = "NIL" ;
      node->m_token.m_content = "nil" ;
    } // else
  } // else if
  else if ( op == ">=" ) {
    float num1, num2 ;
    if ( arg1->m_token.m_type == "INT" )
      num1 = atoi( arg1->m_token.m_content.c_str() ) ;
    else if ( arg1->m_token.m_type == "FLOAT" )
      num1 = atof( arg1->m_token.m_content.c_str() ) ;
    else {
      throw new Our_Exception( "ERROR (>= with incorrect argument type) : ", arg1 ) ;
    } // else

    if ( arg2->m_token.m_type == "INT" )
      num2 = atoi( arg2->m_token.m_content.c_str() ) ;
    else if ( arg2->m_token.m_type == "FLOAT" )
      num2 = atof( arg2->m_token.m_content.c_str() ) ;
    else {
      throw new Our_Exception( "ERROR (>= with incorrect argument type) : ", arg2 ) ;
    } // else

    if ( num1 >= num2 ) {
      node->m_token.m_type = "T" ;
      node->m_token.m_content = "#t" ;
    } // if
    else {
      node->m_token.m_type = "NIL" ;
      node->m_token.m_content = "nil" ;
    } // else
  } // else if
  else if ( op == "<" ) {
    float num1, num2 ;
    if ( arg1->m_token.m_type == "INT" )
      num1 = atoi( arg1->m_token.m_content.c_str() ) ;
    else if ( arg1->m_token.m_type == "FLOAT" )
      num1 = atof( arg1->m_token.m_content.c_str() ) ;
    else {
      throw new Our_Exception( "ERROR (< with incorrect argument type) : ", arg1 ) ;
    } // else

    if ( arg2->m_token.m_type == "INT" )
      num2 = atoi( arg2->m_token.m_content.c_str() ) ;
    else if ( arg2->m_token.m_type == "FLOAT" )
      num2 = atof( arg2->m_token.m_content.c_str() ) ;
    else {
      throw new Our_Exception( "ERROR (< with incorrect argument type) : ", arg2 ) ;
    } // else

    if ( num1 < num2 ) {
      node->m_token.m_type = "T" ;
      node->m_token.m_content = "#t" ;
    } // if
    else {
      node->m_token.m_type = "NIL" ;
      node->m_token.m_content = "nil" ;
    } // else
  } // else if
  else if ( op == "<=" ) {
    float num1, num2 ;
    if ( arg1->m_token.m_type == "INT" )
      num1 = atoi( arg1->m_token.m_content.c_str() ) ;
    else if ( arg1->m_token.m_type == "FLOAT" )
      num1 = atof( arg1->m_token.m_content.c_str() ) ;
    else {
      throw new Our_Exception( "ERROR (<= with incorrect argument type) : ", arg1 ) ;
    } // else

    if ( arg2->m_token.m_type == "INT" )
      num2 = atoi( arg2->m_token.m_content.c_str() ) ;
    else if ( arg2->m_token.m_type == "FLOAT" )
      num2 = atof( arg2->m_token.m_content.c_str() ) ;
    else {
      throw new Our_Exception( "ERROR (<= with incorrect argument type) : ", arg2 ) ;
    } // else


    if ( num1 <= num2 ) {
      node->m_token.m_type = "T" ;
      node->m_token.m_content = "#t" ;
    } // if
    else {
      node->m_token.m_type = "NIL" ;
      node->m_token.m_content = "nil" ;
    } // else
  } // else if
  else if ( op == "=" ) {
    float num1, num2 ;
    if ( arg1->m_token.m_type == "INT" )
      num1 = atoi( arg1->m_token.m_content.c_str() ) ;
    else if ( arg1->m_token.m_type == "FLOAT" )
      num1 = atof( arg1->m_token.m_content.c_str() ) ;
    else {
      throw new Our_Exception( "ERROR (= with incorrect argument type) : ", arg1 ) ;
    } // else

    if ( arg2->m_token.m_type == "INT" )
      num2 = atoi( arg2->m_token.m_content.c_str() ) ;
    else if ( arg2->m_token.m_type == "FLOAT" )
      num2 = atof( arg2->m_token.m_content.c_str() ) ;
    else {
      throw new Our_Exception( "ERROR (= with incorrect argument type) : ", arg2 ) ;
    } // else

    if ( num1 == num2 ) {
      node->m_token.m_type = "T" ;
      node->m_token.m_content = "#t" ;
    } // if
    else {
      node->m_token.m_type = "NIL" ;
      node->m_token.m_content = "nil" ;
    } // else
  } // else if
  else if ( op == "string>?" ) {
    if ( arg1->m_token.m_type != "STRING" ) {
      throw new Our_Exception( "ERROR (string>? with incorrect argument type) : ", arg1 ) ;
    } // if
    else if ( arg2->m_token.m_type != "STRING" ) {
      throw new Our_Exception( "ERROR (string>? with incorrect argument type) : ", arg2 ) ;
    } // else if

    if ( strcmp( arg1->m_token.m_content.c_str(), arg2->m_token.m_content.c_str() ) > 0 ) {
      node->m_token.m_type = "T" ;
      node->m_token.m_content = "#t" ;
    } // if
    else {
      node->m_token.m_type = "NIL" ;
      node->m_token.m_content = "nil" ;
    } // else
  } // else if
  else if ( op == "string<?" ) {
    if ( arg1->m_token.m_type != "STRING" ) {
      throw new Our_Exception( "ERROR (string<? with incorrect argument type) : ", arg1 ) ;
    } // if
    else if ( arg2->m_token.m_type != "STRING" ) {
      throw new Our_Exception( "ERROR (string<? with incorrect argument type) : ", arg2 ) ;
    } // else if

    if ( strcmp( arg1->m_token.m_content.c_str(), arg2->m_token.m_content.c_str() ) < 0 ) {
      node->m_token.m_type = "T" ;
      node->m_token.m_content = "#t" ;
    } // if
    else {
      node->m_token.m_type = "NIL" ;
      node->m_token.m_content = "nil" ;
    } // else
  } // else if
  else if ( op == "string=?" ) {
    if ( arg1->m_token.m_type != "STRING" ) {
      throw new Our_Exception( "ERROR (string=? with incorrect argument type) : ", arg1 ) ;
    } // if
    else if ( arg2->m_token.m_type != "STRING" ) {
      throw new Our_Exception( "ERROR (string=? with incorrect argument type) : ", arg2 ) ;
    } // else if

    if ( arg1->m_token.m_content == arg2->m_token.m_content ) {
      node->m_token.m_type = "T" ;
      node->m_token.m_content = "#t" ;
    } // if
    else {
      node->m_token.m_type = "NIL" ;
      node->m_token.m_content = "nil" ;
    } // else
  } // else if

  return node ;
} // Execute_Arithmetic()

Node* Execute_S_exp( Node* head ) {
  // Atoms at leaf ex. ( function-name a b )
  //                   ( function-name 1 2 )
  // the a and b is a atom->symbol, we need to get its content
  // the 1 and 2 is a atom->int, we just need to return it
  if ( Is_Atom( head ) ) {
    if ( head->m_token.m_type == "SYMBOL" ) {
      bool found = false ;
      for ( int i = 0 ; i < g_SymbolList.size() ; i++ ) {
        if ( head->m_token.m_content == g_SymbolList[i].m_name ) {
          head = g_SymbolList[i].m_head ;
          found = true ;
          i = g_SymbolList.size() ;
        } // if
      } // for

      if ( !found )
        throw new Our_Exception( "ERROR (unbound symbol) : " + head->m_token.m_content, NULL ) ;
    } // if

    return head ;
  } // if
  else if ( head->m_left->m_token.m_content == "cons" ||
            head->m_left->m_token.m_content == "#<procedure cons>" ) {
    // detect non-list error
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    // insufficient arguments ex. ( cons ) , ( cons 1 )
    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type == "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : cons", NULL ) ;
    // too many arguments ex. ( cons 1 2 3 4 )
    else if ( head->m_right->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : cons", NULL ) ;

    Node* temp = new Node() ;
    temp->m_left = Execute_S_exp( head->m_right->m_left ) ;
    temp->m_right = Execute_S_exp( head->m_right->m_right->m_left ) ;
    return temp ;
  } // else if
  else if ( head->m_left->m_token.m_content == "list" ||
            head->m_left->m_token.m_content == "#<procedure list>" ) {
    // empty list ex. ( list )
    if ( head->m_right->m_token.m_type == "NIL" )
      return head->m_right ;
    // detect non-list error
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    Node* new_head = new Node() ;
    Node* temp = new_head ;
    temp->m_left = Execute_S_exp( head->m_right->m_left ) ;
    while ( head->m_right->m_right->m_token.m_type != "NIL" ) {
      temp->m_right = new Node() ;
      temp->m_right->m_left = Execute_S_exp( head->m_right->m_right->m_left ) ;
      head = head->m_right ;
      temp = temp->m_right ;
    } // while

    return new_head ;
  } // else if
  else if ( head->m_left->m_token.m_content == "quote" ||
            head->m_left->m_token.m_content == "#<procedure quote>" ) {
    // detect non-list error
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : quote", NULL ) ;
    return head->m_right->m_left ;
  } // else if
  else if ( head->m_left->m_token.m_content == "define" ||
            head->m_left->m_token.m_content == "#<procedure define>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    // insufficient arguments in define ex. ( define ) , (define a )
    if ( ( head->m_right != NULL && head->m_right->m_token.m_type == "NIL" ) ||
         ( head->m_right->m_right != NULL &&
           head->m_right->m_right->m_token.m_type == "NIL" ) )
      throw new Our_Exception( "ERROR (DEFINE format) : ", NULL ) ;
    // too many arguments in define ex. ( define a 123 234 345 )
    else if ( head->m_right->m_right->m_right != NULL &&
              head->m_right->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (DEFINE format) : ", NULL ) ;
    // level of define is incorrect
    else if ( !head->m_start )
      throw new Our_Exception( "ERROR (level of DEFINE)", NULL ) ;

    // redefinition of existing symbols
    bool found = false ;
    for ( int i = 0 ; i < g_SymbolList.size() ; i++ ) {
      if ( g_SymbolList[i].m_name == head->m_right->m_left->m_token.m_content ) {
        found = true ;
        // tries to redefine system primitive symbol ex. ( define cons 123 )
        if ( !g_SymbolList[i].m_overwrite )
          throw new Our_Exception( "ERROR (DEFINE format) : ", NULL ) ;
        // redefines user defined symbols
        else {
          g_SymbolList[i].m_head = Execute_S_exp( head->m_right->m_right->m_left ) ;
          cout << head->m_right->m_left->m_token.m_content << " defined" << endl ;
        } // else

        // i = g_SymbolList.size() ;
      } // if
    } // for

    // definition of new symbol
    if ( !found ) {
      // trying to define a S_exp ex. ( define ( cons 1 2 ) 123 ) , ( define ( define a 1 ) 2 )
      if ( head->m_right->m_left->m_token.m_content == "" ) {
        Execute_S_exp( head->m_right->m_left ) ;
        throw new Our_Exception( "ERROR (DEFINE format) : ", NULL ) ;
      } // if
      // trying to define a system primitive atom. ex. ( define 123 "hi" )
      else if ( head->m_right->m_left->m_token.m_type  != "SYMBOL" )
        throw new Our_Exception( "ERROR (DEFINE format) : ", NULL ) ;
      // "actually" definition of new symbol
      else {
        Symbol temp( head->m_right->m_left->m_token.m_content, true,
                     Execute_S_exp( head->m_right->m_right->m_left ) ) ;
        g_SymbolList.push_back( temp ) ;
        cout << head->m_right->m_left->m_token.m_content << " defined" << endl ;
      } // else
    } // if
  } // else if
  else if ( head->m_left->m_token.m_content == "car" ||
            head->m_left->m_token.m_content == "#<procedure car>" ) {
    // detect non-list error
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    // insufficient or too many arguments. ex. ( car ) , ( car 1 2 3 )
    if ( head->m_right->m_token.m_type == "NIL" || head->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : car", NULL ) ;

    Node* new_head = Execute_S_exp( head->m_right->m_left ) ;
    if ( new_head->m_left == NULL )
      throw new Our_Exception( "ERROR (car with incorrect argument type) : ", new_head ) ;

    return new_head->m_left ;
  } // else if
  else if ( head->m_left->m_token.m_content == "cdr" ||
            head->m_left->m_token.m_content == "#<procedure cdr>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" || head->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : cdr", NULL ) ;

    Node* new_head = Execute_S_exp( head->m_right->m_left ) ;
    if ( new_head->m_right == NULL )
      throw new Our_Exception( "ERROR (cdr with incorrect argument type) : ", new_head ) ;

    return new_head->m_right ;
  } // else if
  else if ( head->m_left->m_token.m_content == "atom?" ||
            head->m_left->m_token.m_content == "#<procedure atom?>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : atom?", NULL ) ;

    Node* node = new Node() ;
    if ( Is_Atom( Execute_S_exp( head->m_right->m_left ) ) &&
         head->m_right->m_right->m_token.m_type == "NIL" ) {
      node->m_token.m_content = "#t" ;
      node->m_token.m_type = "T" ;
    } // if
    else {
      node->m_token.m_content = "nil" ;
      node->m_token.m_type = "NIL" ;
    } // else

    return node ;
  } // else if
  else if ( head->m_left->m_token.m_content == "pair?" ||
            head->m_left->m_token.m_content == "#<procedure pair?>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : pair?", NULL ) ;
    Node* node = new Node() ;
    Node* temp_pair  = Execute_S_exp( head->m_right->m_left ) ;
    if ( temp_pair->m_token.m_content == "" &&
         temp_pair->m_left != NULL &&
         temp_pair->m_right != NULL ) {
      node->m_token.m_content = "#t" ;
      node->m_token.m_type = "T" ;
    } // if
    else {
      node->m_token.m_content = "nil" ;
      node->m_token.m_type = "NIL" ;
    } // else

    return node ;
  } // else if
  else if ( head->m_left->m_token.m_content == "list?" ||
            head->m_left->m_token.m_content == "#<procedure list?>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : list?", NULL ) ;

    Node* new_head = new Node() ;
    new_head->m_token.m_content = "#t" ;
    new_head->m_token.m_type = "T" ;
    Node* temp = Execute_S_exp( head->m_right->m_left ) ;
    while ( temp->m_token.m_type != "NIL" ) {
      if ( temp->m_token.m_content != "" ) {
        new_head->m_token.m_content = "nil" ;
        new_head->m_token.m_type = "NIL" ;
        temp->m_token.m_type = "NIL" ;
      } // if
      else
        temp = temp->m_right ;
    } // while

    return new_head ;
  } // else if
  else if ( head->m_left->m_token.m_content == "null?" ||
            head->m_left->m_token.m_content == "#<procedure null?>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : null?", NULL ) ;
    Node* node = new Node() ;
    if ( Execute_S_exp( head->m_right->m_left ) ->m_token.m_type == "NIL" &&
         head->m_right->m_right->m_token.m_type == "NIL" ) {
      node->m_token.m_content = "#t" ;
      node->m_token.m_type = "T" ;
    } // if
    else {
      node->m_token.m_content = "nil" ;
      node->m_token.m_type = "NIL" ;
    } // else

    return node ;
  } // else if
  else if ( head->m_left->m_token.m_content == "integer?" ||
            head->m_left->m_token.m_content == "#<procedure integer?>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : integer?", NULL ) ;
    Node* node = new Node() ;
    if ( Execute_S_exp( head->m_right->m_left )->m_token.m_type == "INT" &&
         head->m_right->m_right->m_token.m_type == "NIL" ) {
      node->m_token.m_content = "#t" ;
      node->m_token.m_type = "T" ;
    } // if
    else {
      node->m_token.m_content = "nil" ;
      node->m_token.m_type = "NIL" ;
    } // else

    return node ;
  } // else if
  else if ( head->m_left->m_token.m_content == "number?" || head->m_left->m_token.m_content == "real?" ||
            head->m_left->m_token.m_content == "#<procedure number?>" ||
            head->m_left->m_token.m_content == "#<procedure real?>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_left->m_token.m_content == "number?" &&
         ( head->m_right->m_token.m_type == "NIL" ||
           head->m_right->m_right->m_token.m_type != "NIL" ) )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : number?", NULL ) ;
    else if ( head->m_left->m_token.m_content == "real?" &&
              ( head->m_right->m_token.m_type == "NIL" ||
                head->m_right->m_right->m_token.m_type != "NIL" ) )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : real?", NULL ) ;

    Node* node = new Node() ;
    if ( ( Execute_S_exp( head->m_right->m_left )->m_token.m_type == "FLOAT" ||
           Execute_S_exp( head->m_right->m_left )->m_token.m_type == "INT" ) &&
         head->m_right->m_right->m_token.m_type == "NIL" ) {
      node->m_token.m_content = "#t" ;
      node->m_token.m_type = "T" ;
    } // if
    else {
      node->m_token.m_content = "nil" ;
      node->m_token.m_type = "NIL" ;
    } // else

    return node ;
  } // else if
  else if ( head->m_left->m_token.m_content == "string?" ||
            head->m_left->m_token.m_content == "#<procedure string?>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : string?", NULL ) ;

    Node* node = new Node() ;
    if ( Execute_S_exp( head->m_right->m_left )->m_token.m_type == "STRING" &&
         head->m_right->m_right->m_token.m_type == "NIL" ) {
      node->m_token.m_content = "#t" ;
      node->m_token.m_type = "T" ;
    } // if
    else {
      node->m_token.m_content = "nil" ;
      node->m_token.m_type = "NIL" ;
    } // else

    return node ;
  } // else if
  else if ( head->m_left->m_token.m_content == "boolean?" ||
            head->m_left->m_token.m_content == "#<procedure boolean?>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : boolean?", NULL ) ;

    Node* temp = Execute_S_exp( head->m_right->m_left ) ;
    Node* node = new Node() ;
    if ( ( temp->m_token.m_type == "NIL" || temp->m_token.m_type == "T" ) &&
         head->m_right->m_right->m_token.m_type == "NIL" ) {
      node->m_token.m_content = "#t" ;
      node->m_token.m_type = "T" ;
    } // if
    else {
      node->m_token.m_content = "nil" ;
      node->m_token.m_type = "NIL" ;
    } // else

    return node ;
  } // else if
  else if ( head->m_left->m_token.m_content == "symbol?" ||
            head->m_left->m_token.m_content == "#<procedure symbol?>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : symbol?", NULL ) ;

    Node* node = new Node() ;
    if ( Execute_S_exp( head->m_right->m_left )->m_token.m_type == "SYMBOL" &&
         head->m_right->m_right->m_token.m_type == "NIL" ) {
      node->m_token.m_content = "#t" ;
      node->m_token.m_type = "T" ;
    } // if
    else {
      node->m_token.m_content = "nil" ;
      node->m_token.m_type = "NIL" ;
    } // else

    return node ;
  } // else if
  else if ( head->m_left->m_token.m_content == "+" || head->m_left->m_token.m_content == "-" ||
            head->m_left->m_token.m_content == "*" || head->m_left->m_token.m_content == "/" ||
            head->m_left->m_token.m_content == "and" || head->m_left->m_token.m_content == "or" ||
            head->m_left->m_token.m_content == "#<procedure +>" ||
            head->m_left->m_token.m_content == "#<procedure ->" ||
            head->m_left->m_token.m_content == "#<procedure *>" ||
            head->m_left->m_token.m_content == "#<procedure />" ||
            head->m_left->m_token.m_content == "#<procedure and>" ||
            head->m_left->m_token.m_content == "#<procedure or>" ) {
    // detect non-list error
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    // insufficient arguments ex. ( + ) , ( + 1 )
    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type == "NIL" ) {
      string type = Token_type_string( head->m_left )  ;
      throw new Our_Exception( "ERROR (incorrect number of arguments) : " + type, NULL ) ;
    } // if

    Node* temp = head->m_right ;
    Node* result = Execute_S_exp( head->m_right->m_left ) ;
    while ( temp->m_right->m_token.m_type != "NIL" ) {
      Node* arg = Execute_S_exp( temp->m_right->m_left ) ;
      result = Execute_Arithmetic( result, arg, Token_type_string( head->m_left ).c_str() ) ;
      if ( ( Token_type_string( head->m_left ) == "and" && result->m_token.m_type == "NIL" ) ||
           ( Token_type_string( head->m_left ) == "or" && result->m_token.m_type == "T" ) )
        return result ;
      temp = temp->m_right ;
    } // while

    return result ;
  } // else if
  else if ( head->m_left->m_token.m_content == "not" ||
            head->m_left->m_token.m_content == "#<procedure not>" ) {
    // detect non-list error
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    // insufficient or too many arguments ex. ( not ) , ( not 1 2 3 )
    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : not", NULL ) ;

    Node* arg1 = Execute_S_exp( head->m_right->m_left ) ;
    return Execute_Arithmetic( arg1, NULL, "not" ) ;
  } // else if
  else if ( head->m_left->m_token.m_content == ">" || head->m_left->m_token.m_content == ">=" ||
            head->m_left->m_token.m_content == "<" || head->m_left->m_token.m_content == "<=" ||
            head->m_left->m_token.m_content == "=" ||
            head->m_left->m_token.m_content == "#<procedure >>" ||
            head->m_left->m_token.m_content == "#<procedure >=>" ||
            head->m_left->m_token.m_content == "#<procedure <>" ||
            head->m_left->m_token.m_content == "#<procedure <=>" ||
            head->m_left->m_token.m_content == "#<procedure =>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type == "NIL" ) {
      string type = Token_type_string( head->m_left ) ;
      throw new Our_Exception( "ERROR (incorrect number of arguments) : " + type, NULL ) ;
    } // if

    // check all arguments to be INT or FLOAT
    dummy = head ;
    while ( dummy->m_right->m_token.m_type != "NIL" ) {
      Node* t_arg = Execute_S_exp( dummy->m_right->m_left ) ;
      if ( t_arg->m_token.m_type != "INT" && t_arg->m_token.m_type != "FLOAT" )
        throw new Our_Exception( "ERROR (" + Token_type_string( head->m_left ) +
                                 " with incorrect argument type) : ", t_arg ) ;
      dummy = dummy->m_right ;
    } // while

    // Execution
    Node* result = new Node() ;
    Node* temp = head->m_right ;
    Node* arg1 = Execute_S_exp( head->m_right->m_left ) ;
    while ( temp->m_right->m_token.m_type != "NIL" ) {
      Node* arg2 = Execute_S_exp( temp->m_right->m_left ) ;
      result = Execute_Arithmetic( arg1, arg2, Token_type_string( head->m_left ).c_str() ) ;
      if ( result->m_token.m_type == "NIL" )
        return result ;
      arg1 = arg2 ;
      temp = temp->m_right ;
    } // while

    return result ;
  } // else if
  else if ( head->m_left->m_token.m_content == "string>?" ||
            head->m_left->m_token.m_content == "string<?" ||
            head->m_left->m_token.m_content == "string=?" ||
            head->m_left->m_token.m_content == "#<procedure string>?>" ||
            head->m_left->m_token.m_content == "#<procedure string<?>" ||
            head->m_left->m_token.m_content == "#<procedure string=?>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type == "NIL" ) {
      string type = Token_type_string( head->m_left ) ;
      throw new Our_Exception( "ERROR (incorrect number of arguments) : " + type, NULL ) ;
    } // if

    // check all arguments to be STRING and also save arguments into vector
    vector<Node*> arg_vec ;
    Node* arg = new Node() ;
    dummy = head ;
    while ( dummy->m_right->m_token.m_type != "NIL" ) {
      arg = Execute_S_exp( dummy->m_right->m_left ) ;
      arg_vec.push_back( arg ) ;
      if ( arg->m_token.m_type != "STRING" )
        throw new Our_Exception( "ERROR (" + Token_type_string( head->m_left ) +
                                 " with incorrect argument type) : ", arg ) ;
      dummy = dummy->m_right ;
    } // while

    // Execution
    int arg_vec_index = 0 ;
    Node* result = new Node() ;
    Node* temp = head->m_right ;
    Node* arg1 = arg_vec[arg_vec_index] ;
    while ( temp->m_right->m_token.m_type != "NIL" ) {
      arg_vec_index++ ;
      Node* arg2 = arg_vec[arg_vec_index] ;
      result = Execute_Arithmetic( arg1, arg2, Token_type_string( head->m_left ).c_str() ) ;
      if ( result->m_token.m_type == "NIL" )
        return result ;
      arg1 = arg2 ;
      temp = temp->m_right ;
    } // while

    return result ;
  } // else if
  else if ( head->m_left->m_token.m_content == "string-append" ||
            head->m_left->m_token.m_content == "#<procedure string-append>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type == "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : string-append", NULL ) ;

    Node* result = Execute_S_exp( head->m_right->m_left ) ;
    if ( result->m_token.m_type != "STRING" )
      throw new Our_Exception( "ERROR (string-append with incorrect argument type) : ", result ) ;

    Node* temp = head->m_right ;
    while ( temp->m_right->m_token.m_type != "NIL" ) {
      Node* arg = Execute_S_exp( temp->m_right->m_left ) ;
      if ( arg->m_token.m_type != "STRING" ) {
        throw new Our_Exception( "ERROR (string-append with incorrect argument type) : ", arg ) ;
      } // if

      result->m_token.m_content.insert( result->m_token.m_content.end()-1,
                                        arg->m_token.m_content.begin()+1,
                                        arg->m_token.m_content.end()-1 ) ;
      temp = temp->m_right ;
    } // while

    return result ;
  } // else if
  else if ( head->m_left->m_token.m_content == "eqv?" ||
            head->m_left->m_token.m_content == "#<procedure eqv?>" ) {
    // detect non-list error
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    // insufficient arguments ex. ( eqv? ) , ( eqv? '123 )
    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type == "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : eqv?", NULL ) ;
    // too many arguments ex. ( eqv? 1 2 3 4 )
    else if ( head->m_right->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : eqv?", NULL ) ;

    Node* arg1 = Execute_S_exp( head->m_right->m_left ) ;
    Node* arg2 = Execute_S_exp( head->m_right->m_right->m_left ) ;

    Node* node = new Node() ;
    if (  arg1 == arg2 ) {
      node->m_token.m_content = "#t" ;
      node->m_token.m_type = "T" ;
      return node ;
    } // if
    else if ( Is_Atom( arg1 ) && Is_Atom( arg2 ) && Token_type_string( arg1 ) == Token_type_string( arg2 ) &&
              arg1->m_token.m_type != "STRING" && arg2->m_token.m_type != "STRING" ) {
      node->m_token.m_content = "#t" ;
      node->m_token.m_type = "T" ;
    } // else if
    else {
      node->m_token.m_content = "nil" ;
      node->m_token.m_type = "NIL" ;
    } // else

    return node ;
  } // else if
  else if ( head->m_left->m_token.m_content == "equal?" ||
            head->m_left->m_token.m_content == "#<procedure equal?>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type == "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : equal?", NULL ) ;
    else if ( head->m_right->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : equal?", NULL ) ;

    Node* node = new Node() ;
    Node* arg1 = Execute_S_exp( head->m_right->m_left ) ;
    Node* arg2 = Execute_S_exp( head->m_right->m_right->m_left ) ;
    if ( Test_Equal( arg1, arg2 ) ) {
      node->m_token.m_content = "#t" ;
      node->m_token.m_type = "T" ;
    } // if
    else {
      node->m_token.m_content = "nil" ;
      node->m_token.m_type = "NIL" ;
    } // else

    return node ;
  } // else if
  else if ( head->m_left->m_token.m_content == "begin" ||
            head->m_left->m_token.m_content == "#<procedure begin>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type == "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : begin", NULL ) ;

    Node* new_head ;
    Node* temp = head ;
    while ( temp->m_right->m_token.m_type != "NIL" ) {
      temp = temp->m_right ;
      new_head = Execute_S_exp( temp->m_left ) ;
    } // while

    return new_head ;
  } // else if
  else if ( head->m_left->m_token.m_content == "if" ||
            head->m_left->m_token.m_content == "#<procedure if>" ) {
    // detect non-list error
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    // insufficient arguments ex. ( if ) , ( if #t )
    if ( head->m_right->m_token.m_type == "NIL" ||
         head->m_right->m_right->m_token.m_type == "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : if", NULL ) ;
    // too many arguments ex. ( if 1 2 3 4 )
    else if ( head->m_right->m_right->m_right->m_right != NULL &&
              head->m_right->m_right->m_right->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : if", NULL ) ;

    Node* predicate = Execute_S_exp( head->m_right->m_left ) ;
    Node* expression ;
    if ( predicate->m_token.m_type != "NIL" ) {
      expression = Execute_S_exp( head->m_right->m_right->m_left ) ;
    } // if
    else {
      if ( head->m_right->m_right->m_right->m_token.m_type == "NIL" )
        throw new Our_Exception( "ERROR (no return value) : ", head ) ;
      else
        expression = Execute_S_exp( head->m_right->m_right->m_right->m_left ) ;
    } // else

    return expression ;
  } // else if
  else if ( head->m_left->m_token.m_content == "cond" ||
            head->m_left->m_token.m_content == "#<procedure cond>" ) {
    // detect non-list error
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    // insufficient arguments or wrong format
    // ex. ( cond ) , ( cond 1 )
    // ( cond 1 ), 1 is not a list which includes at least a predicate
    if ( head->m_right->m_token.m_type == "NIL" )
      throw new Our_Exception( "ERROR (COND format) : ", NULL ) ;

    // We must first seek each clause and check its format, MUST have predicate and expression
    dummy = head ;
    while ( dummy->m_right->m_token.m_type != "NIL" ) {
      if ( dummy->m_right->m_left->m_token.m_content != "" ||
           dummy->m_right->m_left->m_right->m_token.m_content != "" )
        throw new Our_Exception( "ERROR (COND format) : ", NULL ) ;
      dummy = dummy->m_right ;
    } // while

    // Seek each Clause
    Node* temp = head ;
    while ( temp->m_right->m_token.m_type != "NIL" ) {
      // if the last clause's predicate is else, we must return the expression in that clause
      if ( temp->m_right->m_left->m_left->m_token.m_content == "else" &&
           temp->m_right->m_right->m_token.m_type == "NIL" ) {
        Node* expression ;
        Node* temp2 = temp->m_right->m_left ;
        // if this "else clause" does not have any expression then error occurred
        if ( temp2->m_right->m_token.m_type == "NIL" )
          throw new Our_Exception( "ERROR (COND format) : ", NULL ) ;
        // Seek the else clause list for the last expression
        while ( temp2->m_right->m_token.m_type != "NIL" ) {
          expression = Execute_S_exp( temp2->m_right->m_left ) ;
          temp2 = temp2->m_right ;
        } // while

        return expression ;
      } // if
      else {
        Node* predicate = Execute_S_exp( temp->m_right->m_left->m_left ) ;
        if ( predicate->m_token.m_type != "NIL" ) {
          Node* expression ;
          Node* temp2 = temp->m_right->m_left ;
          if ( temp2->m_right->m_token.m_type == "NIL" ) {
            throw new Our_Exception( "ERROR (COND format) : ", NULL ) ;
            // In reality, if in this clause we only have predicate and no expression
            // then we return the result of the predicate as the result
            // expression = predicate ;
            // so, yeah we follow the rules of prof. and throw exception if no expression
          } // if
          // Seek this clause list for the last expression
          while ( temp2->m_right->m_token.m_type != "NIL" ) {
            expression = Execute_S_exp( temp2->m_right->m_left ) ;
            temp2 = temp2->m_right ;
          } // while

          // if this clause's predicate is not "NIL"
          // then we return the Expression
          // else we seek for the next clause
          return expression ;
        } // if
        else
          temp = temp->m_right ;
      } // else
    } // while

    throw new Our_Exception( "ERROR (no return value) : ", head ) ;
  } // else if
  else if ( head->m_left->m_token.m_content == "clean-environment" ||
            head->m_left->m_token.m_content == "#<procedure clean-environment>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( head->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : clean-environment", NULL ) ;
    else if ( !head->m_start )
      throw new Our_Exception( "ERROR (level of CLEAN-ENVIRONMENT)", NULL ) ;

    Initialize_Symbols() ;
    cout << "environment cleaned" << endl ;
  } // else if
  else if ( head->m_left->m_token.m_content == "exit" ||
            head->m_left->m_token.m_content == "#<procedure exit>" ) {
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    if ( !head->m_start )
      throw new Our_Exception( "ERROR (level of EXIT)", NULL ) ;
    else if ( head->m_right->m_token.m_type != "NIL" )
      throw new Our_Exception( "ERROR (incorrect number of arguments) : exit", NULL ) ;

    cout << endl << "Thanks for using OurScheme!" ;
    exit( 0 ) ;
  } // else if
  // Execution of User defined Functions or Process error of function names
  else {
    // check for non-list
    Node* dummy = head ;
    while ( dummy->m_token.m_type != "NIL" ) {
      if ( dummy->m_token.m_content != "" ) {
        cout << "ERROR (non-list) : " ;
        Print_Start( head ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      dummy = dummy->m_right ;
    } // while

    // Execute User defined Function Name
    if ( head->m_left->m_token.m_type == "SYMBOL" ) {
      for ( int i = 0 ; i < g_SymbolList.size() ; i++ ) {
        if ( head->m_left->m_token.m_content == g_SymbolList[i].m_name ) {
          Node* temp = new Node() ;
          if ( head->m_start )
            temp->m_start = true ;
          temp->m_left = g_SymbolList[i].m_head ;
          temp->m_right = head->m_right ;
          return Execute_S_exp( temp ) ;
        } // if
      } // for

      string type = Token_type_string( head->m_left ) ;
      throw new Our_Exception( "ERROR (unbound symbol) : " + type, NULL ) ;
    } // if
    // Execute S_Exp as Function name. ex. ( ( cons 1 2 ) 1 2 )
    else if ( head->m_left->m_token.m_type == "" ) {
      Node* temp = new Node() ;
      temp->m_left = Execute_S_exp( head->m_left ) ;
      if ( !Is_Atom( temp->m_left ) ) {
        cout << "ERROR (attempt to apply non-function) : " ;
        Print_Start( temp->m_left ) ;
        throw new Our_Exception( "", NULL ) ;
      } // if

      // allow define to work in left layer ex. ( ( begin define ) a 123 )
      if ( head->m_start )
        temp->m_start = true ;
      temp->m_right = head->m_right ;
      return Execute_S_exp( temp ) ;
    } // else if
    // Execution without Function name at front error. ex. ( 1 2 3 )
    else {
      string type =  Token_type_string( head->m_left ) ;
      throw new Our_Exception( "ERROR (attempt to apply non-function) : " + type, NULL ) ;
    } // else
  } // else

  return NULL ;
} // Execute_S_exp()

void Test( Node* head ) {
  if ( head == NULL ) return ;
  else {
    if ( head->m_token.m_content != "" )
      cout << head->m_token.m_content << endl ;
    else
      cout << "*" << endl ;
    Test( head->m_left ) ;
    Test( head->m_right ) ;
  } // else
} // Test()

int main() {
  int uTestNum ;
  cin >> uTestNum ;
  Initialize_Symbols() ;
  cout << "Welcome to OurScheme!" << endl ;
  cout << endl << "> " ;
  Token token ;
  token = Retrieve_token() ;
  do {
    Node* head ;
    try {
      S_exp( token ) ;
      int bk_line = g_line ;
      int bk_column = g_column ;
      head = Tree_S_exp() ;
      head->m_start = true ;
      Print_Start( Execute_S_exp( head ) ) ;
      cout << endl << "> " ;
      token = Retrieve_token() ;
      if ( token.m_line == bk_line ) {
        g_line = 1 ;
        token.m_line = g_line ;
        g_column = token.m_column - bk_column ;
        token.m_column = g_column ;
      } // if
      else {
        g_line = token.m_line - bk_line ;
        token.m_line = g_line ;
      } // else
    }  catch ( Our_Exception* e ) {
      bool clear_rest_line = false ;
      if ( token.m_content == "" ) {
        cout << "ERROR (no more input) : END-OF-FILE encountered" << endl << "Thanks for using OurScheme!" ;
        exit( 0 ) ;
      } // if

      if ( e->m_msg == "ERROR (unexpected token) : ')' expected when token at" ||
           e->m_msg == "ERROR (unexpected token) : atom or '(' expected when token at" ) {
        clear_rest_line = true ;
        cout << e->m_msg << " Line " << token.m_line << " Column " << token.m_column << " is >>"
             << token.m_content << "<<" << endl ;
      } // if
      else if ( e->m_msg == "ERROR (no closing quote) : END-OF-LINE encountered at" ) {
        clear_rest_line = true ;
        cout << e->m_msg << " Line " << token.m_line
             << " Column " << token.m_column + token.m_string_size << endl ;
      } // else if
      else if ( e->m_msg == "ERROR (DEFINE format) : " ||
                e->m_msg == "ERROR (COND format) : " ) {
        cout << e->m_msg ;
        Print_Start( head ) ;
      } // else if
      else if ( e->m_msg == "" ) ;
      else {
        cout << e->m_msg ;
        Print_Start( e->m_head ) ;
        if ( e->m_head == NULL )
          cout << endl ;
      } // else

      g_TokenList.clear() ;
      if ( clear_rest_line ) {
        char peek ;
        peek = cin.peek() ;
        while ( peek != '\n' && peek != EOF ) {
          peek = getchar() ;
          peek = cin.peek() ;
        } // while

        g_line = 0, g_column = 0 ;
        token = Retrieve_token() ;
      } // if
      else {
        int bk_line = g_line ;
        int bk_column = g_column ;
        token = Retrieve_token() ;
        if ( token.m_line == bk_line ) {
          g_line = 1 ;
          token.m_line = g_line ;
          g_column = token.m_column - bk_column ;
          token.m_column = g_column ;
        } // if
        else {
          g_line = token.m_line - bk_line ;
          token.m_line = g_line ;
        } // else
      } // else

      cout << endl << "> " ;
    } // catch

    g_TokenList.clear() ;
  } while ( token.m_content != "" ) ;

  cout << "ERROR (no more input) : END-OF-FILE encountered" << endl << "Thanks for using OurScheme!" ;
} // main()
