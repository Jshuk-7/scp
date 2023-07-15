#include <iostream>

#include <string>
#include <cstdint>
#include <fstream>
#include <format>
#include <vector>

namespace Scp {

	void SCP_Error(const std::string& msg, const std::string& filename, size_t line, size_t cursor)
	{
		std::cout << std::format("<{}:{}:{}> Error: {}\n", filename, line, cursor, msg);
	}

	class VirtualMachine
	{
	public:
		VirtualMachine()
		{

		}

		~VirtualMachine()
		{

		}

	private:

	};

	enum class TokenType
	{
		Ident,
		Add,
		Sub,
		Mul,
		Div,
		Value,
	};

	namespace Utils {

		std::string TokenTypeToString(TokenType type)
		{
			switch (type)
			{
				case TokenType::Ident: return "Ident";
				case TokenType::Add:   return "Add";
				case TokenType::Sub:   return "Sub";
				case TokenType::Mul:   return "Mul";
				case TokenType::Div:   return "Div";
				case TokenType::Value: return "Value";
			}

			__debugbreak();
			return "";
		}

	}

	struct Token
	{
		std::string Lexeme;
		size_t Line;
		size_t Position;
		TokenType Type;

		void Print() const {
			std::cout << '[' << Utils::TokenTypeToString(Type) << ' ' << Lexeme << ' ' << Line << ':' << Position << "]\n";
		}
	};

	class Lexer
	{
	public:
		Lexer(const std::string& src)
		{
			m_Source = src;
			m_Filename = "script";
			m_Cursor = 0;
			m_TokenStart = 0;
			m_LineStart = 0;
			m_Line = 1;
		}

		void LoadFile(const std::string& filepath)
		{
			std::ifstream file(filepath);
			if (!file.is_open())
			{
				std::string err = "failed to open file: /" + filepath;
				SCP_Error(err, filepath, 0, 0);
				return;
			}

			std::string src;
			file.seekg(0, std::ios::end);
			size_t file_size = file.tellg();
			file.seekg(0, std::ios::beg);
			src.resize(file_size);
			file.read(src.data(), file_size);

			*this = Lexer(src);
			m_Filename = filepath;
		}

		void ScanTokens()
		{
			while (!IsAtEnd())
			{
				Trim();

				m_TokenStart = m_Cursor;

				char first = Current();

				std::string operators = "+-*/";

				if (operators.find(first) != std::string::npos)
				{
					Operator();
				}
				else if (std::isdigit(first))
				{
					Number();
				}
				else if (std::isalpha(first))
				{
					Identifier();
				}
				else if (first == '"')
				{
					String();
				}
				else
				{
					SCP_Error(std::format("unknown symbol '{}'", first), m_Filename, m_Line, m_Cursor);
					return;
				}
			}
		}

		auto GetTokens() { return m_Tokens; }

	private:
		char Advance()
		{
			if (IsAtEnd())
			{
				return '\0';
			}

			++m_Cursor;
			return Current();
		}

		char Current() const
		{
			return m_Source[m_Cursor];
		}

		void Trim()
		{
			while (std::isspace(Current())) {
				if (Current() == '\n') {
					++m_Line;
				}

				Advance();
			}
		}

		std::string LastLexeme() const
		{
			return m_Source.substr(m_TokenStart, m_Cursor - m_TokenStart);
		}

		void Operator()
		{
			char op = Current();
			Advance();

			switch (op) {
				case '+': MakeToken(TokenType::Add); break;
				case '-': MakeToken(TokenType::Sub); break;
				case '*': MakeToken(TokenType::Mul); break;
				case '/': MakeToken(TokenType::Div); break;
				default:
					// unreachable
					break;
			}
		}

		void Identifier()
		{
			while (std::isalnum(Current())) {
				Advance();
			}
			MakeToken(TokenType::Value);
		}

		void Number()
		{
			while (std::isdigit(Current())) {
				Advance();
			}
			MakeToken(TokenType::Value);
		}

		void String()
		{
			Advance();
			m_TokenStart += 1;
			while (std::isalpha(Current()) || std::string(",!@#$%^&*()_+=-/\\[]{}|?><,=.`~;:'").find(Current()) != std::string::npos) {
				Advance();
				Trim();
			}

			if (IsAtEnd() || Current() != '"') {
				SCP_Error("unterminated string literal", m_Filename, m_Line, m_Cursor);
				return;
			}

			MakeToken(TokenType::Value);
			Advance();
		}

		void MakeToken(TokenType type)
		{
			Token token;
			token.Lexeme = LastLexeme();
			token.Line = m_Line;
			token.Position = m_TokenStart;
			token.Type = type;
			m_Tokens.push_back(token);
		}

		bool IsAtEnd() const
		{
			return m_Cursor >= m_Source.size();
		}

	private:
		std::string m_Source;
		std::string m_Filename;
		size_t m_Cursor;
		size_t m_TokenStart;
		size_t m_LineStart;
		size_t m_Line;
		std::vector<Token> m_Tokens;
	};

}

using namespace Scp;

int main()
{
	VirtualMachine vm = VirtualMachine();
	Lexer lexer("1 + 2 \"Hello, World!\"");
	lexer.ScanTokens();
	auto tokens = lexer.GetTokens();
	for (const auto& token : tokens) {
		token.Print();
	}
	std::cin.get();
	return 0;
}