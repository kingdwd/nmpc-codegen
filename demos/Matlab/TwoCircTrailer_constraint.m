close all;
clear all;
addpath(genpath('../../src_matlab'));
%%
step_size=0.03;

% Q and R matrixes determined by the control engineer.
Q = diag([1. 1. 0.0])*0.2;
R = diag([1. 1.]) * 0.01;

Q_terminal = diag([1. 1. 0.0])*2;
R_terminal = diag([1. 1.]) * 0.01;

controller_folder_name = 'demo_controller_matlab';
trailer_controller = prepare_demo_trailer(controller_folder_name,step_size,Q,R,Q_terminal,R_terminal);

trailer_controller.horizon = 30; % NMPC parameter
trailer_controller.integrator_casadi = true; % optional  feature that can generate the integrating used  in the cost function
trailer_controller.panoc_max_steps = 500; % the maximum amount of iterations the PANOC algorithm is allowed to do.
trailer_controller.min_residual=-3;

% construct left circle
left_circle = nmpccodegen.controller.obstacles.Obstacle_circular([0.2; 0.2],0.2);

% construct right circle
right_circle = nmpccodegen.controller.obstacles.Obstacle_circular([0.7; 0.2], 0.2);

% add obstacles to controller
trailer_controller = trailer_controller.add_constraint(left_circle);
trailer_controller = trailer_controller.add_constraint(right_circle);

% experimental feature !!!! this will activate the Lagrangian !
max_speed = 1.5;
max_speed_constraint = nmpccodegen.controller.constraints.Input_norm(max_speed);
trailer_controller = trailer_controller.add_general_constraint(max_speed_constraint);


trailer_controller.shooting_mode='single shot';

trailer_controller.shift_input=true;

% generate the dynamic code
trailer_controller = trailer_controller.generate_code();
%%
% simulate everything
initial_state = [0.2; 0.6; 0];
reference_state = [0.7; -0.02; pi/2];
reference_input = [0; 0];

obstacle_weights = [1000.; 1000.];

[ state_history,time_history,iteration_history,sim ] = simulate_demo_trailer(trailer_controller,initial_state,...
    reference_state,reference_input,obstacle_weights);
clear sim;
%% plot everything TODO make proper plot !
figure;

subplot(2,1,1);
left_circle.plot();
hold on;
right_circle.plot();
nmpccodegen.example_models.trailer_printer(state_history,0.03,'red');
title('path trailer');
xlabel('X coordinate');
ylabel('Y coordinate');

subplot(2,1,2);
plot(iteration_history);
title('amount of iterations till convergence');
xlabel('index simulation step');
ylabel('amount of iterations');