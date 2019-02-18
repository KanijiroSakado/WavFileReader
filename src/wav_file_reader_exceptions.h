#ifdef WAV_FILE_READER_EXCEPTIONS_H
#else
#define WAV_FILE_READER_EXCEPTIONS_H

#include<exception>

namespace gold {

	class WFRException :public std::exception {

	};

	class WFRFileOpenException :public  WFRException {
		const char* what(void) const noexcept {
			return "File Open Failed";
		}
	};

	class WFRFileValidityException :public  WFRException {
		const char* what(void) const noexcept {
			return "Invalid File";
		}
	};

	class WFRSupportedFormatException :public  WFRException {
		const char* what(void) const noexcept {
			return "Unsupported format";
		}
	};

	class WFRReadExption :public std::exception {
	};

	class WFRIllegalArgumentType :public WFRException {
		const char* what(void) const noexcept {
			return "Type of the argument of Read or ReadLR function is illegal";
		}
	};

}

#endif