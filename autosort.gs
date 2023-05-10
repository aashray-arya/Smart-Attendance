function autoSort(e) {
  
  const row= e.range.getRow()
  if(row<2) return

  const ss=SpreadsheetApp.openById('17y4yXqZ-N1bCokzLQ-4vvvuf68_3x7UYajc1D6ztwPo')
  const ws=ss.getSheetByName("main")
  const range= ws.getRange(2,1,ws.getLastRow()-1,3)

  range.sort({column: 3, ascending: true})
}

function onEdit(e){
 autoSort(e)
}
