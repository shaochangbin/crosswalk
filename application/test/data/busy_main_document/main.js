setTimeout(
  function(){
    window.open("main.html");
  }, 100);

setTimeout(busy, 500);

function busy() {
  var prev_millisec = new Date().getTime();
  var curr_millisec = prev_millisec;

  while(true) {
    curr_millisec = new Date().getTime();
    if (curr_millisec - prev_millisec > 2000)
      break;
  }
}

