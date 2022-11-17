var ss = SpreadsheetApp.openById('1aLEZErlQ4l2eY5sievtCRpg2CWrwMZsaWuJUERgNwiI');
var sheet = ss.getSheetByName('Sheet1');

function doGet(e){
  var read = e.parameter.read;

  if (read !== undefined){
    return ContentService.createTextOutput(sheet.getRange('C3').getValue());
  }
}
