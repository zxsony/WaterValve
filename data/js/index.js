
function requeryTemp () {

  const pars1 = new URLSearchParams({
  val:  1
  });

  fetch('/temp?' + pars1 )                // Do the request
  .then(response => response.text())    // Parse the response 
  .then(text => {                       // DO something with response
    document.getElementById('t1').innerHTML = text;
  });
  
  const pars2 = new URLSearchParams({
  val:  2
  });

  fetch('/temp?' + pars2 )                // Do the request
  .then(response => response.text())    // Parse the response 
  .then(text => {                       // DO something with response
    document.getElementById('t2').innerHTML = text;
  });

}

setInterval(requeryTemp, 1000);