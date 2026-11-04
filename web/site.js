// Theme toggle (remembers choice) and back-to-top button.
(function(){
  var root=document.documentElement,btn=document.getElementById('theme'),top=document.getElementById('totop');
  function cur(){return root.getAttribute('data-theme')||(matchMedia('(prefers-color-scheme:dark)').matches?'dark':'light');}
  function label(){btn.textContent=cur()==='dark'?'Light mode':'Dark mode';}
  try{var s=localStorage.getItem('theme');if(s)root.setAttribute('data-theme',s);}catch(e){}
  label();
  btn.addEventListener('click',function(){var n=cur()==='dark'?'light':'dark';root.setAttribute('data-theme',n);try{localStorage.setItem('theme',n);}catch(e){}label();});
  addEventListener('scroll',function(){top.classList.toggle('show',scrollY>500);});
  top.addEventListener('click',function(){scrollTo({top:0});});
})();
