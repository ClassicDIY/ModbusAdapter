#include <IotWebConf.h>

const char CUSTOMHTML_SCRIPT_INNER[] PROGMEM = "\n\
function mySelect(selectObject) {\n\
  var value = selectObject.value; \n\
  console.log(value); \n\
  document.getElementById('rtuBaudRateInputParam').value = value; \n\
};\n";
// -- HTML element will be added inside the body element.
const char CUSTOMHTML_BODY_INNER[] PROGMEM = "<div class><input type='number' id='rtuBaudRateInputParam' name='rtuBaudRateInputParam'><label for='baudSelector'>RTU Baud Rate </label><select id='baudSelector' name='baudSelector' onChange='mySelect(this.options[this.selectedIndex])'>\n\
  <option value='9600'>9600</option>\n\
  <option value='19200'>19200</option>\n\
</select><div class='em'></div></div>\n";


class CustomHtmlFormatProvider : public IotWebConfHtmlFormatProvider
{
protected:
  String getScriptInner() override
  {
    return
      IotWebConfHtmlFormatProvider::getScriptInner() +
      String(FPSTR(CUSTOMHTML_SCRIPT_INNER));
  }
};