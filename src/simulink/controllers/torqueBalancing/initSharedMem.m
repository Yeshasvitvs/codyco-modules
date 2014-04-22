nj = 25;   %number of joints
nc = 12;   %number of contraints

% Add path to MATLAB
% addpath(genpath('/home/daniele/MATLAB'))
% addpath(genpath('/home/daniele/src/codyco/build'))
% addpath(genpath('/home/daniele/src/codyco/src/simulink'))

% Controller period
Ts = 0.01; 
 

% Controller gains in P I D order
weights = [0.2512      0.3335       0.428      0.9506      0.9604      0.9831      0.9877      0.9974      0.9514      0.9601      0.9829      0.9879      0.9974      0.4056       0.213      0.2676      0.1629      0.0545      0.0108      0.4054       0.213      0.2674      0.1628      0.0545      0.0108];

k_com = [ 20    5   15];
k_pst = [ 100.*diag(1./weights)   0.*diag(1./weights)   0.*diag(1./weights)];
k_pos = [ 100   500   10];
  
kw = 1;  