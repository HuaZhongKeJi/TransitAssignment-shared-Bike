function [S,y] =myquadprog (H,f,A,b,Aeq,beq,lb,ub)
opts = optimoptions('quadprog','Display','off');
[S,y] = quadprog(H,f,A,b,Aeq,beq,lb,ub,[],opts)
end
%function [result] = myadd(a,b)
% result = a + b;
%end