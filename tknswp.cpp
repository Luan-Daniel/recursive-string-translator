#ifndef TKNSWP_CPP
#define TKNSWP_CPP

#include "json/json.cpp"
#include <list>

//#include <vector>
//#include <set>

namespace tknswp{

	namespace tkn{
		std::list<std::string>
		split_string(const std::string &str, const std::list<std::string> &tkn){
			std::list<std::string> res;
		}
	}

	//OLD
	/*
	namespace tools{
		std::string inline
		getFileExtension(std::string const& file){
  			return std::string(file.begin() + file.rfind('.') +1, file.end());
		}

		template<typename T>
		std::vector<T> inline
		setToVector(std::set<T> const& s){
			return std::vector<T>(s.begin(), s.end());
		}

		namespace tkn{
  			std::vector<std::string>
  			strsplt(std::string const& str, std::string const& delim){
				std::string split;
				std::vector<std::string> v;
				size_t i=0, c=0;
				while(i!=std::string::npos){
					i = str.find(delim, c);
					split = str.substr(c, (i==std::string::npos?i:i-c));
					v.emplace_back(split);
					c = i+delim.size();
				}
				return v;
			}

			std::vector<std::string>
			gettkns(std::string const& str, std::string const& form, std::string const& tkn="&$var"){
				std::vector<std::string> const splt = strsplt(form, tkn);
				std::vector<std::string> tkns;
				size_t c1=0, s1=0;
				for(size_t i=0; i<splt.size(); i++){
					c1 = str.find(splt[i], c1+s1)+splt[i].size();
					s1 = (i+1<splt.size()? str.find(splt[i+1], c1) - c1: std::string::npos);
					tkns.emplace_back(str.substr(c1,s1));
				}
				return tkns;
			}

			std::string
			puttkns(std::string const& form, std::vector<std::string> const& tkns, std::string const& tkn="&$var"){
				std::vector<std::string> const splt = strsplt(form, tkn);
				std::string res;
				for(size_t i=0; i<splt.size(); i++){
					res += splt[i];
					if(i<tkns.size()) res += tkns[i];
				}
				return res;
			}
		}
	}

	//translate/enhance interface
	struct Interface{
		const
		std::string
			dirEvaluate = "./lang/evaluate/",
			dirEnhance = "./lang/enhance/";

		std::string
			_lang;

		std::vector<std::string>
			_files;

		json::jsonWrapper
			evaluate,
			enhance;

		bool _loaded_evaluate=false, _loaded_enhance=false;

		Interface(std::vector<std::string> const& files){
			_files = files;
			_lang = "en";
			evaluate.set(json::Value(json::Object()));
			enhance.set(json::Value(json::Object()));
		}

		Interface(std::vector<std::string> const& files, std::string lang){
			_files = files;
			_lang = lang;
			evaluate.set(json::Value(json::Object()));
			enhance.set(json::Value(json::Object()));
		}

		bool
		loadTransLangLib(){
			try{
				evaluate.emplaceFile("en", dirEvaluate + "en.json");
				std::string const filename = dirEvaluate + _lang + ".json";
				std::ifstream file(filename);
				if (file.is_open()){
					std::string line, text;
					while(std::getline(file, line)) text += line;
					file.close();
					std::string::const_iterator c(text.begin());
					evaluate(_lang) = json::parse::parseValue(c, text.end());
				}
				else{
					evaluate(_lang) = evaluate["en"];
				}
				_loaded_evaluate = true;
			}
			catch (json::exception::ParseError &e){
				fprintf(stdout, "json: ParseError: %s\n", e.what());
				_loaded_evaluate = false;
			}
			catch (std::out_of_range &e){
				fprintf(stdout, "json: out_of_range: %s\n", e.what());
				_loaded_evaluate = false;
			}
			return _loaded_evaluate;
		}

		bool
		loadEnhacedLangLib(){
			try{
				for(std::string& _file : _files){
					enhance(_file).emplaceFile("default", dirEnhance + _file + '/' + "default.json");
					std::string const 	filename = dirEnhance + _file + '/' + _lang + ".json";
					std::ifstream file(filename);
					if (file.is_open()){
						std::string line, text;
						while(std::getline(file, line)) text += line;
						file.close();
						std::string::const_iterator c(text.begin());
						enhance(_file)(_lang) = json::parse::parseValue(c, text.end());
						continue;
					}
					enhance(_file)(_lang) = enhance[_file]["default"];
				}
				_loaded_enhance = true;
			}
			catch (json::exception::ParseError &e){
				fprintf(stdout, "json parser error: %s\n", e.what());
				_loaded_enhance = false;
			}
			catch (std::out_of_range &e){
				fprintf(stdout, "json acess error: %s\n", e.what());
				_loaded_enhance = false;
			}

			return _loaded_enhance;
		}

		std::string
		langEvaluate(int const id){
			if(evaluate.find(_lang)){
				if(evaluate.at(_lang).find(to_string(id)))
					return evaluate.at(_lang).at(to_string(id)).get<json::String>();
			}
			else if(evaluate.find("en")){
				if(evaluate.at("en").find(to_string(id)))
					return evaluate.at("en").at(to_string(id)).get<json::String>();
			}
			throw std::out_of_range("langEvaluate error: missing 'id'");
		}

		std::pair<std::string, std::vector<std::string>>
		_getridtkns(std::string const& info, std::string const& file){
			json::Object const df = enhance[file]["default"].get<json::Object>();
			for (auto const& [id, _data] : df){
				auto data = _data->get<json::String>();
				if (data.find("&$var")!=std::string::npos){
					std::vector<std::string> v = tools::tkn::strsplt(data, "&$var");
					bool match = true;
					for(std::string const& s: v){
						if(info.find(s)==std::string::npos){
							std::vector<std::string>().swap(v);
							match = false;
							break;
						}
					}
					if (match){
						std::vector<std::string> rt = tools::tkn::gettkns(info, data);
						return {id, rt};
					}
				}
				else if(info == data) return {id, std::vector<std::string>()};
			}
			return {"", std::vector<std::string>()};
		}

		// WIP
		std::string
		enhanceMessageDiv(std::string const& info, std::string const& file=""){
			std::string const fl = (file==""? _files.at(0): file);
			auto [id, tkn] = _getridtkns(info, fl);
			if(id=="")
				return "<case>" + info;
			if(tkn.size()==0)
				return "<caseEnhanced>" + enhance[fl][_lang][id].get<json::String>() + "<caseOriginal>" + info;
			return "<caseEnhanced>"
				+ tools::tkn::puttkns(
					enhance[fl][_lang][id].get<json::String>(),
					tkn
				)
				+ "<caseOriginal>"
				+ info;
		}

		std::string
		enhanceMessage(std::string const& info, std::string const& file=""){
			std::string const fl = (file==""? _files.at(0): file);
			auto [id, tkn] = _getridtkns(info, fl);
			if(id=="")
				return info;
			if(tkn.size()==0)
				return enhance[fl][_lang][id].get<json::String>();
			return
				tools::tkn::puttkns(
				enhance[fl][_lang][id].get<json::String>(),
				tkn
				);
		}
	};

	Interface* L;
	*/
}

#endif