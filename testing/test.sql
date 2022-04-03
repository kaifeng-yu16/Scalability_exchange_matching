select a.sum + b.sum, c.sum + d.sum 
from (select sum(balance) from account)c, 
     (select sum(price * amount) from sym_order where amount>0 AND status='open')d,  
     (select sum(amount) from position)a, 
     (select sum(-amount) from sym_order where amount<0 AND status='open')b;
