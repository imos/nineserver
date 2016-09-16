#include "nineserver/http/html.h"

#include <algorithm>
#include <string>

namespace {

const map<string, string>& GetHtmlEntities() {
  static map<string, string>* kHtmlEntities = new map<string, string>({
      {"&quot;", "\x22"}, {"&amp;", "\x26"}, {"&lt;", "\x3c"}, {"&gt;", "\x3e"},
      {"&nbsp;", "\xc2\xa0"}, {"&iexcl;", "\xc2\xa1"}, {"&cent;", "\xc2\xa2"},
      {"&pound;", "\xc2\xa3"}, {"&curren;", "\xc2\xa4"}, {"&yen;", "\xc2\xa5"},
      {"&brvbar;", "\xc2\xa6"}, {"&sect;", "\xc2\xa7"}, {"&uml;", "\xc2\xa8"},
      {"&copy;", "\xc2\xa9"}, {"&ordf;", "\xc2\xaa"}, {"&laquo;", "\xc2\xab"},
      {"&not;", "\xc2\xac"}, {"&shy;", "\xc2\xad"}, {"&reg;", "\xc2\xae"},
      {"&macr;", "\xc2\xaf"}, {"&deg;", "\xc2\xb0"}, {"&plusmn;", "\xc2\xb1"},
      {"&sup2;", "\xc2\xb2"}, {"&sup3;", "\xc2\xb3"}, {"&acute;", "\xc2\xb4"},
      {"&micro;", "\xc2\xb5"}, {"&para;", "\xc2\xb6"}, {"&middot;", "\xc2\xb7"},
      {"&cedil;", "\xc2\xb8"}, {"&sup1;", "\xc2\xb9"}, {"&ordm;", "\xc2\xba"},
      {"&raquo;", "\xc2\xbb"}, {"&frac14;", "\xc2\xbc"},
      {"&frac12;", "\xc2\xbd"}, {"&frac34;", "\xc2\xbe"},
      {"&iquest;", "\xc2\xbf"}, {"&Agrave;", "\xc3\x80"},
      {"&Aacute;", "\xc3\x81"}, {"&Acirc;", "\xc3\x82"},
      {"&Atilde;", "\xc3\x83"}, {"&Auml;", "\xc3\x84"}, {"&Aring;", "\xc3\x85"},
      {"&AElig;", "\xc3\x86"}, {"&Ccedil;", "\xc3\x87"},
      {"&Egrave;", "\xc3\x88"}, {"&Eacute;", "\xc3\x89"},
      {"&Ecirc;", "\xc3\x8a"}, {"&Euml;", "\xc3\x8b"},
      {"&Igrave;", "\xc3\x8c"}, {"&Iacute;", "\xc3\x8d"},
      {"&Icirc;", "\xc3\x8e"}, {"&Iuml;", "\xc3\x8f"}, {"&ETH;", "\xc3\x90"},
      {"&Ntilde;", "\xc3\x91"}, {"&Ograve;", "\xc3\x92"},
      {"&Oacute;", "\xc3\x93"}, {"&Ocirc;", "\xc3\x94"},
      {"&Otilde;", "\xc3\x95"}, {"&Ouml;", "\xc3\x96"}, {"&times;", "\xc3\x97"},
      {"&Oslash;", "\xc3\x98"}, {"&Ugrave;", "\xc3\x99"},
      {"&Uacute;", "\xc3\x9a"}, {"&Ucirc;", "\xc3\x9b"}, {"&Uuml;", "\xc3\x9c"},
      {"&Yacute;", "\xc3\x9d"}, {"&THORN;", "\xc3\x9e"},
      {"&szlig;", "\xc3\x9f"}, {"&agrave;", "\xc3\xa0"},
      {"&aacute;", "\xc3\xa1"}, {"&acirc;", "\xc3\xa2"},
      {"&atilde;", "\xc3\xa3"}, {"&auml;", "\xc3\xa4"}, {"&aring;", "\xc3\xa5"},
      {"&aelig;", "\xc3\xa6"}, {"&ccedil;", "\xc3\xa7"},
      {"&egrave;", "\xc3\xa8"}, {"&eacute;", "\xc3\xa9"},
      {"&ecirc;", "\xc3\xaa"}, {"&euml;", "\xc3\xab"}, {"&igrave;", "\xc3\xac"},
      {"&iacute;", "\xc3\xad"}, {"&icirc;", "\xc3\xae"}, {"&iuml;", "\xc3\xaf"},
      {"&eth;", "\xc3\xb0"}, {"&ntilde;", "\xc3\xb1"}, {"&ograve;", "\xc3\xb2"},
      {"&oacute;", "\xc3\xb3"}, {"&ocirc;", "\xc3\xb4"},
      {"&otilde;", "\xc3\xb5"}, {"&ouml;", "\xc3\xb6"},
      {"&divide;", "\xc3\xb7"}, {"&oslash;", "\xc3\xb8"},
      {"&ugrave;", "\xc3\xb9"}, {"&uacute;", "\xc3\xba"},
      {"&ucirc;", "\xc3\xbb"}, {"&uuml;", "\xc3\xbc"}, {"&yacute;", "\xc3\xbd"},
      {"&thorn;", "\xc3\xbe"}, {"&yuml;", "\xc3\xbf"}, {"&OElig;", "\xc5\x92"},
      {"&oelig;", "\xc5\x93"}, {"&Scaron;", "\xc5\xa0"},
      {"&scaron;", "\xc5\xa1"}, {"&Yuml;", "\xc5\xb8"}, {"&fnof;", "\xc6\x92"},
      {"&circ;", "\xcb\x86"}, {"&tilde;", "\xcb\x9c"}, {"&Alpha;", "\xce\x91"},
      {"&Beta;", "\xce\x92"}, {"&Gamma;", "\xce\x93"}, {"&Delta;", "\xce\x94"},
      {"&Epsilon;", "\xce\x95"}, {"&Zeta;", "\xce\x96"}, {"&Eta;", "\xce\x97"},
      {"&Theta;", "\xce\x98"}, {"&Iota;", "\xce\x99"}, {"&Kappa;", "\xce\x9a"},
      {"&Lambda;", "\xce\x9b"}, {"&Mu;", "\xce\x9c"}, {"&Nu;", "\xce\x9d"},
      {"&Xi;", "\xce\x9e"}, {"&Omicron;", "\xce\x9f"}, {"&Pi;", "\xce\xa0"},
      {"&Rho;", "\xce\xa1"}, {"&Sigma;", "\xce\xa3"}, {"&Tau;", "\xce\xa4"},
      {"&Upsilon;", "\xce\xa5"}, {"&Phi;", "\xce\xa6"}, {"&Chi;", "\xce\xa7"},
      {"&Psi;", "\xce\xa8"}, {"&Omega;", "\xce\xa9"}, {"&alpha;", "\xce\xb1"},
      {"&beta;", "\xce\xb2"}, {"&gamma;", "\xce\xb3"}, {"&delta;", "\xce\xb4"},
      {"&epsilon;", "\xce\xb5"}, {"&zeta;", "\xce\xb6"}, {"&eta;", "\xce\xb7"},
      {"&theta;", "\xce\xb8"}, {"&iota;", "\xce\xb9"}, {"&kappa;", "\xce\xba"},
      {"&lambda;", "\xce\xbb"}, {"&mu;", "\xce\xbc"}, {"&nu;", "\xce\xbd"},
      {"&xi;", "\xce\xbe"}, {"&omicron;", "\xce\xbf"}, {"&pi;", "\xcf\x80"},
      {"&rho;", "\xcf\x81"}, {"&sigmaf;", "\xcf\x82"}, {"&sigma;", "\xcf\x83"},
      {"&tau;", "\xcf\x84"}, {"&upsilon;", "\xcf\x85"}, {"&phi;", "\xcf\x86"},
      {"&chi;", "\xcf\x87"}, {"&psi;", "\xcf\x88"}, {"&omega;", "\xcf\x89"},
      {"&thetasym;", "\xcf\x91"}, {"&upsih;", "\xcf\x92"},
      {"&piv;", "\xcf\x96"}, {"&ensp;", "\xe2\x80\x82"},
      {"&emsp;", "\xe2\x80\x83"}, {"&thinsp;", "\xe2\x80\x89"},
      {"&zwnj;", "\xe2\x80\x8c"}, {"&zwj;", "\xe2\x80\x8d"},
      {"&lrm;", "\xe2\x80\x8e"}, {"&rlm;", "\xe2\x80\x8f"},
      {"&ndash;", "\xe2\x80\x93"}, {"&mdash;", "\xe2\x80\x94"},
      {"&lsquo;", "\xe2\x80\x98"}, {"&rsquo;", "\xe2\x80\x99"},
      {"&sbquo;", "\xe2\x80\x9a"}, {"&ldquo;", "\xe2\x80\x9c"},
      {"&rdquo;", "\xe2\x80\x9d"}, {"&bdquo;", "\xe2\x80\x9e"},
      {"&dagger;", "\xe2\x80\xa0"}, {"&Dagger;", "\xe2\x80\xa1"},
      {"&bull;", "\xe2\x80\xa2"}, {"&hellip;", "\xe2\x80\xa6"},
      {"&permil;", "\xe2\x80\xb0"}, {"&prime;", "\xe2\x80\xb2"},
      {"&Prime;", "\xe2\x80\xb3"}, {"&lsaquo;", "\xe2\x80\xb9"},
      {"&rsaquo;", "\xe2\x80\xba"}, {"&oline;", "\xe2\x80\xbe"},
      {"&frasl;", "\xe2\x81\x84"}, {"&euro;", "\xe2\x82\xac"},
      {"&image;", "\xe2\x84\x91"}, {"&weierp;", "\xe2\x84\x98"},
      {"&real;", "\xe2\x84\x9c"}, {"&trade;", "\xe2\x84\xa2"},
      {"&alefsym;", "\xe2\x84\xb5"}, {"&larr;", "\xe2\x86\x90"},
      {"&uarr;", "\xe2\x86\x91"}, {"&rarr;", "\xe2\x86\x92"},
      {"&darr;", "\xe2\x86\x93"}, {"&harr;", "\xe2\x86\x94"},
      {"&crarr;", "\xe2\x86\xb5"}, {"&lArr;", "\xe2\x87\x90"},
      {"&uArr;", "\xe2\x87\x91"}, {"&rArr;", "\xe2\x87\x92"},
      {"&dArr;", "\xe2\x87\x93"}, {"&hArr;", "\xe2\x87\x94"},
      {"&forall;", "\xe2\x88\x80"}, {"&part;", "\xe2\x88\x82"},
      {"&exist;", "\xe2\x88\x83"}, {"&empty;", "\xe2\x88\x85"},
      {"&nabla;", "\xe2\x88\x87"}, {"&isin;", "\xe2\x88\x88"},
      {"&notin;", "\xe2\x88\x89"}, {"&ni;", "\xe2\x88\x8b"},
      {"&prod;", "\xe2\x88\x8f"}, {"&sum;", "\xe2\x88\x91"},
      {"&minus;", "\xe2\x88\x92"}, {"&lowast;", "\xe2\x88\x97"},
      {"&radic;", "\xe2\x88\x9a"}, {"&prop;", "\xe2\x88\x9d"},
      {"&infin;", "\xe2\x88\x9e"}, {"&ang;", "\xe2\x88\xa0"},
      {"&and;", "\xe2\x88\xa7"}, {"&or;", "\xe2\x88\xa8"},
      {"&cap;", "\xe2\x88\xa9"}, {"&cup;", "\xe2\x88\xaa"},
      {"&int;", "\xe2\x88\xab"}, {"&there4;", "\xe2\x88\xb4"},
      {"&sim;", "\xe2\x88\xbc"}, {"&cong;", "\xe2\x89\x85"},
      {"&asymp;", "\xe2\x89\x88"}, {"&ne;", "\xe2\x89\xa0"},
      {"&equiv;", "\xe2\x89\xa1"}, {"&le;", "\xe2\x89\xa4"},
      {"&ge;", "\xe2\x89\xa5"}, {"&sub;", "\xe2\x8a\x82"},
      {"&sup;", "\xe2\x8a\x83"}, {"&nsub;", "\xe2\x8a\x84"},
      {"&sube;", "\xe2\x8a\x86"}, {"&supe;", "\xe2\x8a\x87"},
      {"&oplus;", "\xe2\x8a\x95"}, {"&otimes;", "\xe2\x8a\x97"},
      {"&perp;", "\xe2\x8a\xa5"}, {"&sdot;", "\xe2\x8b\x85"},
      {"&lceil;", "\xe2\x8c\x88"}, {"&rceil;", "\xe2\x8c\x89"},
      {"&lfloor;", "\xe2\x8c\x8a"}, {"&rfloor;", "\xe2\x8c\x8b"},
      {"&lang;", "\xe2\x8c\xa9"}, {"&rang;", "\xe2\x8c\xaa"},
      {"&loz;", "\xe2\x97\x8a"}, {"&spades;", "\xe2\x99\xa0"},
      {"&clubs;", "\xe2\x99\xa3"}, {"&hearts;", "\xe2\x99\xa5"},
      {"&diams;", "\xe2\x99\xa6"}});
  return *kHtmlEntities;
}

string GetUtf8(int32 code) {
  if (code < 0) {
    return "";
  }
  if (code < 0x80) {
    return string(1, code);
  }
  string buffer;
  for (int i = 1; i <= 6; i++) {
    if (code < (1 << (7 - i))) {
      buffer.append(1, (255 << (8 - i)) | code);
      break;
    }
    buffer.append(1, (code & 0x3f) | 0x80);
    code >>= 6;
  }
  std::reverse(buffer.begin(), buffer.end());
  return buffer;
}

}  // namespace

string EscapeHtml(StringPiece input) {
  string output;
  output.reserve(input.size() + 1);
  for (int i = 0; i < input.size(); i++) {
    switch (input[i]) {
      case '&': output.append("&amp;"); break;
      case '"': output.append("&quot;"); break;
      case '\'': output.append("&apos;"); break;
      case '<': output.append("&lt;"); break;
      case '>': output.append("&gt;"); break;
      default: output.append(1, input[i]); break;
    }
  }
  return output;
}

string UnescapeHtml(StringPiece input) {
  string output;
  output.reserve(input.size() + 1);
  for (int i = 0; i < input.size(); i++) {
    if (input[i] != '&') {
      output.append(1, input[i]);
      continue;
    }
    int start = i;
    for (; i < input.size() && input[i] != ';'; i++);
    string entity(input.data() + start, input.data() + i + 1);
    LOG(ERROR) << entity;
    if (HasPrefixString(entity, "&#")) {
      int32 code = 0;
      if (HasPrefixString(entity, "&#x")) {
        safe_strto32_base(entity.c_str() + 3, &code, 16);
      } else {
        safe_strto32_base(entity.c_str() + 2, &code, 10);
      }
      output.append(GetUtf8(code));
    } else {
      output.append(FindWithDefault(
          GetHtmlEntities(), entity, entity));
    }
  }
  return output;
}
